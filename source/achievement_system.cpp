#include "achievement_system.h"
#include <iostream>


AchievementSystem::AchievementSystem() {
    m_achievements[EnemyType::Bike] = BuildingType::Spikes;
    m_achievements[EnemyType::Truck] = BuildingType::Mine;
    m_achievements[EnemyType::BTR] = BuildingType::Hedgehogs;
    m_achievements[EnemyType::Tank] = BuildingType::AntitankGun;
    m_achievements[EnemyType::CruiserI] = BuildingType::TwinGun;
}

void AchievementSystem::defeated(EnemyType enemy_type) {
    auto it = m_achievements.find(enemy_type);
    if (it != m_achievements.end()) {
        m_unlocked_buildings[it->second] = true;
    }
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
