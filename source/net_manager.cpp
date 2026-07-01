#include <net_manager.h>
#include <params_manager.h>

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
