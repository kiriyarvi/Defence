#pragma once
#include "tile_map.h"
#include "enemies/IEnemy.h"
#include "guns/minigun.h"
#include <functional>
#include <unordered_map>
#include <vector>


struct IStringifier {
public:
    struct Comparation {
        std::string prop;
        std::string current_value;
        std::string goal_value;
    };

    virtual Comparation compare(const std::string& prop, float current, float goal) = 0;
    virtual Comparation compare(const std::string& prop, int current, int goal) = 0;
    virtual ~IStringifier() = default;
};


struct Upgrade {
    Upgrade() = default;
    std::string name;
    std::string general_description;
    int max_level;
    int available_level = 0;
    std::unordered_map<EnemyType, int> upgrade_conditions; //< TODO это можно читать из json.
    virtual void upgrade(IBuilding* building, int goal) = 0;
    virtual int get_current_upgrate(IBuilding* building) = 0;
    virtual std::vector<IStringifier::Comparation> compare(IStringifier* stringifier, IBuilding* building, int goal) = 0;
    std::string get_unlock_condition_description(int level);
    virtual int cost(int level) = 0;
    bool on_event(EnemyType defeated_enemy);
};



struct MinigunPenetrationUpgrade : public Upgrade {
    MinigunPenetrationUpgrade() {
        name = "Бронебойные снаряды";
        general_description = "Пулемет получит снаряды повышенной бронепробиваемости.";
        auto& params = ParamsManager::Instance().params.guns.minigun;
        max_level = params.penetration_upgrades.size() - 1;
        upgrade_conditions = {
            { EnemyType::Pickup, 1 }
        };
    }
 
    void upgrade(IBuilding* building, int goal) override {
        MiniGun* minigun = static_cast<MiniGun*>(building);
        minigun->upgrade_penetration(goal);
    }

    int get_current_upgrate(IBuilding* building) override {
        MiniGun* minigun = static_cast<MiniGun*>(building);
        return minigun->m_penetration_upgrade;
    }

    std::vector<IStringifier::Comparation> compare(IStringifier* stringifier, IBuilding* building, int goal) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        MiniGun* minigun = static_cast<MiniGun*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier->compare(
            "бронепробиваемость при минимальном нагреве",
            params.penetration_upgrades[goal - 1].min_armor_penetration_level,
            params.penetration_upgrades[goal].min_armor_penetration_level
        ));
        out.push_back(stringifier->compare(
            "бронепробиваемость при максимальной нагреве",
            params.penetration_upgrades[goal - 1].max_armor_penetration_level,
            params.penetration_upgrades[goal].max_armor_penetration_level
        ));
        out.push_back(stringifier->compare(
            "минимальный урон",
            params.penetration_upgrades[goal - 1].min_damage,
            params.penetration_upgrades[goal].min_damage
        ));
        out.push_back(stringifier->compare(
            "максимальный урон",
            params.penetration_upgrades[goal - 1].max_damage,
            params.penetration_upgrades[goal].max_damage
        ));
        return out;
    }

    int cost(int level) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        return params.penetration_upgrades[level].cost;
    }
};


struct MinigunCoolingUpgrade : public Upgrade {
    MinigunCoolingUpgrade() {
        name = "Система охлаждения";
        general_description = "Пулемет получит улучшенную систему охлаждения, продливающую время работы при критическом нагреве, а также увеличивающую скорость охлаждения.";
        auto& params = ParamsManager::Instance().params.guns.minigun;
        max_level = params.cooling_upgrades.size() - 1;
        upgrade_conditions = {
            { EnemyType::CruiserI, 1 }
        };
    }

    void upgrade(IBuilding* building, int goal) override {
        MiniGun* minigun = static_cast<MiniGun*>(building);
        minigun->upgrade_cooling(goal);
    }

    int get_current_upgrate(IBuilding* building) override {
        MiniGun* minigun = static_cast<MiniGun*>(building);
        return minigun->m_cooling_upgrade;
    }

    std::vector<IStringifier::Comparation> compare(IStringifier* stringifier, IBuilding* building, int goal) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        MiniGun* minigun = static_cast<MiniGun*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier->compare(
            "время работы при критическом перегреве",
            params.cooling_upgrades[goal - 1].critical_temperature_work_duration,
            params.cooling_upgrades[goal].critical_temperature_work_duration
        ));
        out.push_back(stringifier->compare(
            "время полного охлаждения",
            params.cooling_upgrades[goal - 1].cooling_time,
            params.cooling_upgrades[goal].cooling_time
        ));
        out.push_back(stringifier->compare(
            "время на охлаждение после перегрева",
            params.cooling_upgrades[goal - 1].cooldown_duration,
            params.cooling_upgrades[goal].cooldown_duration
        ));
        return out;
    }

    int cost(int level) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        return params.cooling_upgrades[level].cost;
    }
};

struct MinigunLubricantUpgrade : public Upgrade {
    MinigunLubricantUpgrade() {
        name = "Смазочная система";
        general_description = "Пулемет получит улучшенную смазочную систему, что позволит ему набирать максимальную скорость вращения барабана быстрее. При этом соответсвие скорости и нагрева останется прежним.";
        auto& params = ParamsManager::Instance().params.guns.minigun;
        max_level = params.lubricant_upgrades.size() - 1;
        upgrade_conditions = {
            { EnemyType::CruiserI, 1 }
        };
    }

    void upgrade(IBuilding* building, int goal) override {
        MiniGun* minigun = static_cast<MiniGun*>(building);
        minigun->upgrade_lubricant(goal);
    }

    int get_current_upgrate(IBuilding* building) override {
        MiniGun* minigun = static_cast<MiniGun*>(building);
        return minigun->m_lubricant_upgrade;
    }

    std::vector<IStringifier::Comparation> compare(IStringifier* stringifier, IBuilding* building, int goal) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        MiniGun* minigun = static_cast<MiniGun*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier->compare(
            "Время нагрева до максимальной температуры",
            params.lubricant_upgrades[goal - 1].heating_time,
            params.lubricant_upgrades[goal].heating_time
        ));
        return out;
    }

    int cost(int level) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        return params.lubricant_upgrades[level].cost;
    }
};




class AchievementSystem {
public:
    // Получение единственного экземпляра
    static AchievementSystem& Instance() {
        static AchievementSystem instance; // Создаётся при первом вызове, потокобезопасно в C++11+
        return instance;
    }

    // Удаляем копирование и перемещение
    AchievementSystem(const AchievementSystem&) = delete;
    AchievementSystem& operator=(const AchievementSystem&) = delete;
    AchievementSystem(AchievementSystem&&) = delete;
    AchievementSystem& operator=(AchievementSystem&&) = delete;

    bool defeated(EnemyType enemy_type);
    bool is_unlocked(BuildingType type);
    std::string get_building_unlock_condition_description(BuildingType type);

    //Minigun upgrades
    MinigunPenetrationUpgrade minigun_penetration_upgrade;
    MinigunCoolingUpgrade minigun_cooling_upgrade;
    MinigunLubricantUpgrade minigun_lubricant_upgrade;
    ////Radar upgrades
    //int radar_radius_upgrades = 0;
    //int radar_uncovering_level_upgrades = 0;
    //int radar_uncovering_speed_upgrades = 0;
    //int radar_long_distance_communication_upgrade = 0;

    void unlock_all();
private:
    AchievementSystem();
    std::unordered_map<BuildingType, bool> m_unlocked_buildings;
    std::unordered_map<EnemyType, std::vector<BuildingType>> m_building_unlock_achievements;
    std::vector<Upgrade*> m_upgrades;
};
