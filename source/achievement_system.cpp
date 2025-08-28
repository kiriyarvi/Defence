#include "achievement_system.h"
#include <iostream>


AchievementSystem::AchievementSystem() {
    m_achievements[EnemyType::Bike] = BuildingType::Spikes;
    m_achievements[EnemyType::Truck] = BuildingType::Mine;
    m_achievements[EnemyType::BTR] = BuildingType::Hedgehogs;
    m_achievements[EnemyType::Tank] = BuildingType::AntitankGun;
    m_achievements[EnemyType::CruiserI] = BuildingType::TwinGun;

    m_general_achievements[EnemyType::Pickup] = [&]() {
        minigun_upgrades.penetration_upgrade = 1;
        minigun_upgrades.penetration_upgrade = 3;
        minigun_upgrades.lubricant_update = 3;
        minigun_upgrades.cooling_upgrade = 3;
    };
    m_unlocked_buildings[BuildingType::Minigun] = true;
    m_unlocked_buildings[BuildingType::Mine] = true;
}

bool AchievementSystem::defeated(EnemyType enemy_type) {
    bool achievement = false;
    auto it = m_achievements.find(enemy_type);
    if (it != m_achievements.end()) {
        m_unlocked_buildings[it->second] = true;
        achievement = true;
    }
    auto g_it = m_general_achievements.find(enemy_type);
    if (g_it != m_general_achievements.end()) {
        g_it->second();
        m_general_achievements.erase(g_it);
        achievement = true;
    }
    return achievement;
}

bool AchievementSystem::is_unlocked(BuildingType type) {
    return m_unlocked_buildings[type];
}

std::string AchievementSystem::get_achievement_description(BuildingType type) {
    for (auto [e, a] : m_achievements) {
        if (a == type)
            return "Необходимо победить <b>" + to_string(e) + "</b>";
    }
    return "";
}
