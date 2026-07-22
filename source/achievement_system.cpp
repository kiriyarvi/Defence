#include "achievement_system.h"
#include "game_state.h"

std::string Upgrade::get_unlock_condition_description(int level) {
    for (auto [type, val] : upgrade_conditions) {
        if (val == level)
            return "необходимо уничтожить: " + to_string(type);
    }
    return "cooming soon";
}

bool Upgrade::on_event(EnemyType defeated_enemy) {
    auto it = upgrade_conditions.find(defeated_enemy);
    if (it != upgrade_conditions.end() && it->second >= available_level) {
        available_level = it->second;
        return true;
    }
    return false;
}



AchievementSystem::AchievementSystem() {
    m_upgrades = { &minigun_penetration_upgrade, &minigun_cooling_upgrade, &minigun_lubricant_upgrade };

    m_building_unlock_achievements = {
        {EnemyType::Bike, {BuildingType::Spikes }},
        {EnemyType::Truck,{ BuildingType::Mine }},
        {EnemyType::BTR,{ BuildingType::Hedgehogs }},
        {EnemyType::Tank,{ BuildingType::AntitankGun }},
        {EnemyType::CruiserI, { BuildingType::TwinGun }},
    };
    m_unlocked_buildings[BuildingType::Minigun] = true;
}

bool AchievementSystem::defeated(EnemyType enemy_type) {
    bool update = false;
    auto it = m_building_unlock_achievements.find(enemy_type);
    if (it != m_building_unlock_achievements.end()) {
        update = !m_unlocked_buildings[it->second[0]]; //если первая постройка в списке уже разблокирована, значит достижение уже получено
        if (update) {
            for (auto b : it->second) {
                m_unlocked_buildings[b] = true;
                GameState::Instance().get_ui().get_console()->add_building_unlock_message(b);
            }
        }
    }
    for (auto& upgrade : m_upgrades)
        if (upgrade->on_event(enemy_type)) {
            GameState::Instance().get_ui().get_console()->add_upgrade_unlock_message(*upgrade, upgrade->available_level);
            update = true;
        }
    return update;
}

bool AchievementSystem::is_unlocked(BuildingType type) {
    return m_unlocked_buildings[type];
}

std::string AchievementSystem::get_building_unlock_condition_description(BuildingType type) {
    for (auto [enemy, buildings]: m_building_unlock_achievements) {
        if (std::find(buildings.begin(), buildings.end(), type) != buildings.end())
            return "необхоимо уничтожить: " + to_string(enemy);
    }
    return "cooming soon";
}


void AchievementSystem::unlock_all() {
    m_unlocked_buildings[BuildingType::Minigun] = true;
    m_unlocked_buildings[BuildingType::AntitankGun] = true;
    m_unlocked_buildings[BuildingType::Hedgehogs] = true;
    m_unlocked_buildings[BuildingType::Mine] = true;
    m_unlocked_buildings[BuildingType::Spikes] = true;
    m_unlocked_buildings[BuildingType::TwinGun] = true;
    m_unlocked_buildings[BuildingType::RadioMast] = true;
    m_unlocked_buildings[BuildingType::Radar] = true;
    minigun_penetration_upgrade.available_level = minigun_penetration_upgrade.max_level;
    //minigun_cooling_upgrade.available_level = minigun_cooling_upgrade.max_level;
    //minigun_lubricant_upgrade.available_level = minigun_lubricant_upgrade.max_level;
    radar_radius_upgrade.available_level = radar_radius_upgrade.max_level;
    radar_uncovering_level_upgrade.available_level = radar_uncovering_level_upgrade.max_level;
    radar_uncovering_speed_upgrade.available_level = radar_uncovering_speed_upgrade.max_level;
    radar_long_distance_communication_upgrade.available_level = radar_long_distance_communication_upgrade.max_level;
}
