#include "net_manager.h"
#include "params_manager.h"
#include "enemy_manager.h"
#include "covering_database.h"
#include "guns/radar.h"


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

static bool in_radio_tower_zone(NetManager::Net::CellID radio_tower_cell_id, const glm::vec2& pos) {
    static auto& params = ParamsManager::Instance().params.guns.radio_tower;
    return glm::length(glm::vec2(pos.x - 32 * radio_tower_cell_id.x_id - 16, pos.y - 32 * radio_tower_cell_id.y_id - 16)) <= 32 * params.radius;
}

static bool in_radio_tower_zone(NetManager::Net::CellID radio_tower_cell_id, NetManager::Net::CellID radar_id) {
    static auto& params = ParamsManager::Instance().params.guns.radio_tower;
    return glm::length(glm::vec2(radar_id.x_id - radio_tower_cell_id.x_id, radar_id.y_id - radio_tower_cell_id.y_id)) <= params.radius;
}

bool NetManager::Net::in_radio_tower_zone(const glm::vec2& pos) const {
    auto& params = ParamsManager::Instance().params.guns.radio_tower;
    for (auto& tower : radio_towers) {
        if (::in_radio_tower_zone(tower, pos))
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
    std::list<EnemyContainer::EnemyID> busy_targets;
    for (auto& radar : radars)
        for (auto& target : radar.radar_ptr->m_targets)
            busy_targets.push_back(target.target);

    // C. Составляем список врагов, попадающих в зону раскрытия сети
    std::list<IEnemy*> enemies_to_uncover;
    for (auto enemy : enemy_manager.get_enemy_container()) {
        if (CoveringDataBase::Instance().is_available_taget(enemy->id))
            continue; //цель рассекречена => пропускаем
        if (std::find_if(busy_targets.begin(), busy_targets.end(), [&](EnemyContainer::EnemyID target) {
            return target == enemy->id;
        }) != busy_targets.end())
            continue; //цель уже раскрывается одним из радаров
        if (!in_uncover_zone(enemy->position))
            continue; //враг не в зона действия сети
        enemies_to_uncover.push_back(enemy);
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

void NetManager::radar_deleted(int x_id, int y_id) {
    //Определить сеть, в которую входит радар и удалить его из этой сети
    m_radar_register.erase(std::find(m_radar_register.begin(), m_radar_register.end(), Net::RadarInfo{ {x_id, y_id}, nullptr }));
    for (auto& net : m_nets) {
        for (auto it = net.radars.begin(); it != net.radars.end(); ++it) {
            if (it->cell.x_id == x_id && it->cell.y_id == y_id) {
                net.radars.erase(it);
                return;
            }
        }
    }
}

void NetManager::radio_tower_deleted(int x_id, int y_id) {
    //A. Найти сеть, в которую входит вышка и удалить вышку из сети
    std::list<Net>::iterator net = m_nets.end();
    for (auto net_it = m_nets.begin(); net_it != m_nets.end(); ++net_it) {
        for (auto radio_tower_it = net_it->radio_towers.begin(); radio_tower_it != net_it->radio_towers.end(); ++radio_tower_it) {
            if (radio_tower_it->x_id == x_id && radio_tower_it->y_id == y_id) {
                net = net_it;
                net->radio_towers.erase(radio_tower_it);
                break;
            }
        }
        if (net != m_nets.end())
            break;
    }
    assert(net != m_nets.end());
    //B Удалили последнюю вышку сети => удалили сеть
    if (net->radio_towers.empty()) {
        for (auto& radar : net->radars)
            radar.radar_ptr->m_part_of_net = false;
        m_nets.erase(net);
    }
    else {
        //C Пересобрать сеть.
        //1. Удалим старую сеть.
        Net old_net = std::move(*net);
        m_nets.erase(net);
        //2. Объединяем вышки в сеть
        std::list<Net> new_nets;
        while (!old_net.radio_towers.empty()) {
            bool create_new_net = false;
            if (new_nets.empty())
                create_new_net = true;
            else {
                if (std::none_of(old_net.radio_towers.begin(), old_net.radio_towers.end(), [&](Net::CellID id) {
                    return new_nets.back().in_radio_tower_zone({ id.x_id * 32 + 16, id.y_id * 32 + 16 });
                })) //ни одна из вышек не в зоне последней созданной сети.
                    create_new_net = true;
            }

            if (create_new_net) {
                Net new_net;
                new_net.radio_towers.push_back(old_net.radio_towers.front());
                old_net.radio_towers.pop_front();
                new_nets.push_back(new_net);
            }
            bool filled = false;
            auto& filling_net = new_nets.back();
            for (auto radio_tower_it = old_net.radio_towers.begin(); radio_tower_it != old_net.radio_towers.end();) {
                if (filling_net.in_radio_tower_zone({ radio_tower_it->x_id * 32 + 16, radio_tower_it->y_id * 32 + 16 })) {
                    filling_net.radio_towers.push_back(*radio_tower_it);
                    radio_tower_it = old_net.radio_towers.erase(radio_tower_it);
                    filled = true;
                }
                else
                    ++radio_tower_it;
            }
            if (!filled && !old_net.radio_towers.empty()) { //исчерпали данную сеть, начинаем новую, если еще есть вышки
                Net new_net;
                new_net.radio_towers.push_back(old_net.radio_towers.front());
                old_net.radio_towers.pop_front();
                new_nets.push_back(new_net);
            }
        }
        //3. Добавляем радары
        for (auto& radar : old_net.radars) {
            bool radar_in_net = false;
            for (auto& net : new_nets) {
                if (net.in_radio_tower_zone({ radar.cell.x_id * 32 + 16, radar.cell.y_id * 32 + 16 })) {
                    net.radars.push_back(radar);
                    radar_in_net = true;
                    break;
                }
            }
            if (!radar_in_net)
                radar.radar_ptr->m_part_of_net = false;
        }
        //4. Добавляем новые сети.
        m_nets.splice(m_nets.end(), std::move(new_nets));
    }
}
