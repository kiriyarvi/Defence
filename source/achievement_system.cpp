#include "achievement_system.h"
#include "game_state.h"

EnemyDefeatedCondition::EnemyDefeatedCondition(EnemyType type) : m_type{type} {}

std::string EnemyDefeatedCondition::get_description() {
    return "необходимо уничтожить " + to_string(m_type);
}

bool EnemyDefeatedCondition::defeated(EnemyType enemy_type) {
    return enemy_type == m_type;
}

AchievementSystem::MinigunUpgrades AchievementSystem::minigun_upgrades{};

Condition enemy_defaeted_condition(EnemyType enemy_type) {
    return std::make_unique<EnemyDefeatedCondition>(enemy_type);
}

Achievement building_for_enemy_achievement(EnemyType enemy, BuildingType building) {
    Achievement a;
    a.condition = enemy_defaeted_condition(enemy);
    a.unlocked_buildings.push_back(building);
    return a;
}


Upgrade AchievementSystem::MinigunUpgrades::create_penetration_upgrade(int n) {
    return Upgrade("Бронебойные снаряды", BuildingType::Minigun, &AchievementSystem::minigun_upgrades.penetration_upgrade, n);
}

Upgrade AchievementSystem::MinigunUpgrades::create_lubricant_upgrade(int n) {
    return Upgrade("Смазка", BuildingType::Minigun, &AchievementSystem::minigun_upgrades.lubricant_update, n);
}

Upgrade AchievementSystem::MinigunUpgrades::create_cooling_upgrade(int n) {
    return Upgrade("Система охлаждения", BuildingType::Minigun, &AchievementSystem::minigun_upgrades.cooling_upgrade, n);
}


AchievementSystem::AchievementSystem() {
    m_achievements.push_back(building_for_enemy_achievement(EnemyType::Bike, BuildingType::Spikes));

    Achievement pickup;
    pickup.condition = enemy_defaeted_condition(EnemyType::Pickup);
    pickup.upgrades.push_back(MinigunUpgrades::create_penetration_upgrade(1));
    m_achievements.push_back(std::move(pickup));

    m_achievements.push_back(building_for_enemy_achievement(EnemyType::Truck, BuildingType::Mine));
    m_achievements.push_back(building_for_enemy_achievement(EnemyType::BTR, BuildingType::Hedgehogs));
    m_achievements.push_back(building_for_enemy_achievement(EnemyType::Tank, BuildingType::AntitankGun));
    m_achievements.push_back(building_for_enemy_achievement(EnemyType::CruiserI, BuildingType::TwinGun));

    Achievement cruiserI;
    cruiserI.condition = enemy_defaeted_condition(EnemyType::CruiserI);
    cruiserI.upgrades.push_back(MinigunUpgrades::create_penetration_upgrade(2));
    cruiserI.upgrades.push_back(MinigunUpgrades::create_lubricant_upgrade(1));
    cruiserI.upgrades.push_back(MinigunUpgrades::create_cooling_upgrade(1));
    m_achievements.push_back(std::move(cruiserI));
    
    m_unlocked_buildings[BuildingType::Minigun] = true;
}

bool AchievementSystem::defeated(EnemyType enemy_type) {
    bool achievement = false;
    auto it = m_achievements.begin();
    while (it != m_achievements.end()) {
        if (it->condition->defeated(enemy_type)) {
            for (auto& b : it->unlocked_buildings) {
                m_unlocked_buildings[b] = true;
            }
            for (auto& up : it->upgrades) {
                *up.upgrade_variable = up.goal_value;
            }
            it = m_achievements.erase(it);
            return true;
        }
        ++it;
    }
    return false;
}

bool AchievementSystem::is_unlocked(BuildingType type) {
    return m_unlocked_buildings[type];
}

std::string AchievementSystem::get_building_unlock_condition_description(BuildingType type) {
    for (auto& a : m_achievements) {
        if (std::find(a.unlocked_buildings.begin(), a.unlocked_buildings.end(), type) != a.unlocked_buildings.end())
            return a.condition->get_description();
    }
    return "cooming soon";
}

std::string AchievementSystem::get_upgrade_unlock_condition_description(int* var, int goal_value) {
    for (auto& a : m_achievements) {
        for (auto& up : a.upgrades) {
            if (up.upgrade_variable == var && up.goal_value == goal_value) {
                return a.condition->get_description();
            }
        }
    }
    return "cooming soon";
}


void AchievementSystem::unlock_all() {
    m_unlocked_buildings[BuildingType::AntitankGun] = true;
    m_unlocked_buildings[BuildingType::Hedgehogs] = true;
    m_unlocked_buildings[BuildingType::Mine] = true;
    m_unlocked_buildings[BuildingType::Minigun] = true;
    m_unlocked_buildings[BuildingType::Spikes] = true;
    m_unlocked_buildings[BuildingType::TwinGun] = true;
    minigun_upgrades.cooling_upgrade = 3;
    minigun_upgrades.lubricant_update = 3;
    minigun_upgrades.penetration_upgrade = 3;
}
