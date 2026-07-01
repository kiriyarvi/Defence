#include "net_manager.h"
#include "params_manager.h"
#include "enemy_manager.h"
#include "covering_database.h"
#include "guns/radar.h"

NetManager::NetManager() {

}

bool NetManager::Net::CellID::operator==(const CellID& cell) const {
    return this->x_id == cell.x_id && this->y_id == cell.y_id;
}

bool NetManager::Net::RadarInfo::operator==(const RadarInfo& radar) const {
    return this->cell == radar.cell;
}


bool NetManager::Net::in_uncover_zone(const glm::vec2& pos) const {
    auto& params = ParamsManager::Instance().params.guns.radar;
    for (auto& r_info : radars) {
        int radius = params.radius_upgrades[r_info.radar_ptr->radius_upgrade].radius;
        if (glm::distance(pos, glm::vec2{ 32 * r_info.cell.x_id + 16, 32 * r_info.cell.y_id + 16 }) <= 32 * radius)
            return true;
    }
    return false;
}

bool NetManager::Net::in_radio_tower_zone(const glm::vec2& pos) const {
    auto& params = ParamsManager::Instance().params.guns.radio_tower;
    for (auto& tower : radio_towers) {
        if (glm::length(glm::vec2(pos.x - 32 * tower.x_id - 16, pos.y - 32 * tower.y_id - 16)) <= 32 * params.radius)
            return true;
    }
    return false;
}


void NetManager::Net::logic(float dtime_microseconds) {

    auto& params = ParamsManager::Instance().params.guns.radar;
    auto& enemy_manager = EnemyManager::Instance();

    // A. Внутренняя логика радаров, раскрытие целей.
    for (auto& radar_info : radars) {
        auto& radar = *radar_info.radar_ptr;
        float aiming_time = params.uncovering_speed_upgrades[radar.uncovering_speed_upgrade].aiming_time;
        float uncover_time = params.uncovering_speed_upgrades[radar.uncovering_speed_upgrade].uncover_time;
        float radius = params.radius_upgrades[radar.radius_upgrade].radius;
        float uncovering_level = params.uncovering_level_upgrades[radar.uncovering_level_upgrade].uncovering_level;
        int max_targets = params.max_targets;

        // 1. Таймер наведения
        if (radar.m_status == Radar::Status::Aiming) {
            radar.m_aiming_timer += dtime_microseconds;
            if (radar.m_aiming_timer >= aiming_time * 1000.f * 1000.f)
                radar.m_status = Radar::Status::Uncover;
        }

        // 2. удаляем несуществующие цели + увеличиваем таймер. Удаляем, если цель раскрыта
        for (auto it = radar.m_targets.begin(); it != radar.m_targets.end();) {
            if (!enemy_manager.get_enemy_by_id(it->target)) { // враг больше не существует. TODO это место очень узкое
                it = radar.m_targets.erase(it);
                continue;
            }
            auto enemy_status = CoveringDataBase::Instance().get_status(it->target);
            if (enemy_status.uncovering_level >= enemy_status.covering_level) { // враг раскрыт (возможно другой сетью).
                it = radar.m_targets.erase(it);
                continue;
            }
            it->uncovering_time += dtime_microseconds;
            if (it->uncovering_time >= uncover_time * 1000 * 1000) {
                CoveringDataBase::Instance().enemy_uncovering_level(it->target, uncovering_level);
                it = radar.m_targets.erase(it);
            }
            else
                ++it;
        }
    }
    // B. Собираем актуальный список раскрывающихся целей
    std::list<int> busy_targets;
    for (auto& radar : radars)
        for (auto& target : radar.radar_ptr->m_targets)
            busy_targets.push_back(target.target);

    // C. Составляем список врагов, попадающих в зону раскрытия сети
    std::list<IEnemy*> enemies_to_uncover;
    for (auto& enemy : enemy_manager.m_enemies) {
        if (CoveringDataBase::Instance().is_available_taget(enemy->id))
            continue; //цель рассекречена => пропускаем
        if (std::find_if(busy_targets.begin(), busy_targets.end(), [&](int target) {
            return target == enemy->id;
        }) != busy_targets.end())
            continue; //цель уже раскрывается одним из радаров
        if (!in_uncover_zone(enemy->position))
            continue; //враг не в зона действия сети
        enemies_to_uncover.push_back(enemy.get());
    }
    enemies_to_uncover.sort([](IEnemy* a, IEnemy* b) {
        return a->path_progress < b->path_progress;
    });

    //D. Назначаем цели радарам. Нужно назначить цели с наименьшим e->path_progress
    for (auto& radar_info : radars) {
        auto& radar = *radar_info.radar_ptr;
        if (radar.m_status == Radar::Status::Aiming)
            continue;
        if (radar.m_targets.size() >= params.max_targets)
            continue;
        float uncovering_level =
            params.uncovering_level_upgrades[radar.uncovering_level_upgrade].uncovering_level;

        for (auto it = enemies_to_uncover.begin();it != enemies_to_uncover.end();++it){
            if ((*it)->m_covering_level > uncovering_level)
                continue;

            radar.m_status = Radar::Status::Aiming;
            radar.m_aiming_timer = 0;
            radar.m_targets.push_back({ (*it)->id, 0.0 });

            enemies_to_uncover.erase(it);
            break;
        }
    }
}

void NetManager::new_radar(int x_id, int y_id, Radar* radar) {
    m_radar_register.push_back({ Net::CellID{x_id, y_id}, radar });
    for (auto& n : m_nets) {
        if (n.in_radio_tower_zone({ x_id * 32 + 16, y_id * 32 + 16 })) {
            n.radars.push_back({ Net::CellID{x_id, y_id}, radar });
            radar->m_part_of_net = true;
            return;
        }
    }
}


void NetManager::new_radio_tower(int x_id, int y_id) {
    std::list<Net::RadarInfo> radars_in_zone;
    auto& params = ParamsManager::Instance().params.guns.radio_tower;
    for (auto& radar : m_radar_register) {
        if (glm::distance(glm::vec2(radar.cell.x_id, radar.cell.y_id), glm::vec2(x_id, y_id)) <= params.radius) {
            radars_in_zone.push_back(radar);
            radar.radar_ptr->m_part_of_net = true;
        }
    }
    // Все сети, которых касается новая вышка
    std::vector<std::list<Net>::iterator> nets_to_merge;

    for (auto it = m_nets.begin(); it != m_nets.end(); ++it) {
        if (it->in_radio_tower_zone(glm::vec2(32 * x_id + 16, 32 * y_id + 16)))
            nets_to_merge.push_back(it);
    }

    // Создаем общую сеть
    Net new_net;
    new_net.radars = std::move(radars_in_zone);
    new_net.radio_towers.push_back({ x_id, y_id });

    for (auto& net : nets_to_merge) {
        for (auto& r : net->radars) {
            if (std::find(new_net.radars.begin(), new_net.radars.end(), r) == new_net.radars.end())
                new_net.radars.push_back(r);
        }
        for (auto& t : net->radio_towers)
            new_net.radio_towers.push_back(t);
    }
    //удалить nets_to_merge и добавить new_net
    for (auto it : nets_to_merge)
        m_nets.erase(it);
    m_nets.push_back(std::move(new_net));
}

void NetManager::logic(float dtime_microseconds) {
    for (auto& net : m_nets)
        net.logic(dtime_microseconds);
}


const NetManager::Net& NetManager::get_net_by_radiotower(Net::CellID id) const {
    for (auto& net : m_nets) {
        for (auto& tower : net.radio_towers) {
            if (tower == id)
                return net;
        }
    }
}
