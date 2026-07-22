#pragma once
#include "tile_map.h"
#include "enemies/IEnemy.h"
#include "guns/minigun.h"
#include "guns/radar.h"
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
    BuildingType building_type;
    std::string name;
    std::string general_description;
    int max_level;
    int available_level = 0;
    std::unordered_map<EnemyType, int> upgrade_conditions; //< TODO это можно читать из json.
    virtual void upgrade(IBuilding* building, int goal) = 0;
    virtual int get_current_upgrate(IBuilding* building) = 0;
    virtual std::vector<IStringifier::Comparation> compare(IStringifier& stringifier, IBuilding* building, int level) = 0;
    std::string get_unlock_condition_description(int level);
    virtual int cost(int level) = 0;
    bool on_event(EnemyType defeated_enemy);
    virtual std::string get_name(int level) const { return name + " " + std::string(level, 'I'); }
};



struct MinigunPenetrationUpgrade : public Upgrade {
    MinigunPenetrationUpgrade() {
        building_type = BuildingType::Minigun;
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

    std::vector<IStringifier::Comparation> compare(IStringifier& stringifier, IBuilding* building, int level) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        MiniGun* minigun = static_cast<MiniGun*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier.compare(
            "бронепробиваемость при минимальном нагреве",
            params.penetration_upgrades[level - 1].min_armor_penetration_level,
            params.penetration_upgrades[level].min_armor_penetration_level
        ));
        out.push_back(stringifier.compare(
            "бронепробиваемость при максимальной нагреве",
            params.penetration_upgrades[level - 1].max_armor_penetration_level,
            params.penetration_upgrades[level].max_armor_penetration_level
        ));
        out.push_back(stringifier.compare(
            "минимальный урон",
            params.penetration_upgrades[level - 1].min_damage,
            params.penetration_upgrades[level].min_damage
        ));
        out.push_back(stringifier.compare(
            "максимальный урон",
            params.penetration_upgrades[level - 1].max_damage,
            params.penetration_upgrades[level].max_damage
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

    std::vector<IStringifier::Comparation> compare(IStringifier& stringifier, IBuilding* building, int level) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        MiniGun* minigun = static_cast<MiniGun*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier.compare(
            "время работы при критическом перегреве",
            params.cooling_upgrades[level - 1].critical_temperature_work_duration,
            params.cooling_upgrades[level].critical_temperature_work_duration
        ));
        out.push_back(stringifier.compare(
            "время полного охлаждения",
            params.cooling_upgrades[level - 1].cooling_time,
            params.cooling_upgrades[level].cooling_time
        ));
        out.push_back(stringifier.compare(
            "время на охлаждение после перегрева",
            params.cooling_upgrades[level - 1].cooldown_duration,
            params.cooling_upgrades[level].cooldown_duration
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

    std::vector<IStringifier::Comparation> compare(IStringifier& stringifier, IBuilding* building, int level) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        MiniGun* minigun = static_cast<MiniGun*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier.compare(
            "Время нагрева до максимальной температуры",
            params.lubricant_upgrades[level - 1].heating_time,
            params.lubricant_upgrades[level].heating_time
        ));
        return out;
    }

    int cost(int level) override {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        return params.lubricant_upgrades[level].cost;
    }
};

struct RadarRadiusUpgrade : public Upgrade {
    RadarRadiusUpgrade() {
        name = "Радиус обнаружения";
        general_description = "Радар получить более совершенное оборудование, позволяющее раскрывать противников на большем расстоянии.";
        auto& params = ParamsManager::Instance().params.guns.radar;
        max_level = params.radius_upgrades.size() - 1;
        upgrade_conditions = {};
    }

    void upgrade(IBuilding* building, int goal) override {
        Radar* radar = static_cast<Radar*>(building);
        radar->upgrade_radius(goal);
    }

    int get_current_upgrate(IBuilding* building) override {
        Radar* radar = static_cast<Radar*>(building);
        return radar->radius_upgrade;
    }

    std::vector<IStringifier::Comparation> compare(IStringifier& stringifier, IBuilding* building, int level) override {
        auto& params = ParamsManager::Instance().params.guns.radar;
        Radar* radar = static_cast<Radar*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier.compare(
            "Радиус обнаружения",
            params.radius_upgrades[level - 1].radius,
            params.radius_upgrades[level].radius
        ));
        return out;
    }

    int cost(int level) override {
        auto& params = ParamsManager::Instance().params.guns.radar;
        return params.radius_upgrades[level].cost;
    }
};


struct RadarUncoveringLevelUpgrade : public Upgrade {
    RadarUncoveringLevelUpgrade() {
        name = "Система радиоподавления";
        general_description = "Радар будет обнаруживать противников с более высоким уровнем маскировки.";
        auto& params = ParamsManager::Instance().params.guns.radar;
        max_level = params.uncovering_level_upgrades.size() - 1;
        upgrade_conditions = {};
    }

    void upgrade(IBuilding* building, int goal) override {
        Radar* radar = static_cast<Radar*>(building);
        radar->upgrade_uncovering_level(goal);
    }

    int get_current_upgrate(IBuilding* building) override {
        Radar* radar = static_cast<Radar*>(building);
        return radar->uncovering_level_upgrade;
    }

    std::vector<IStringifier::Comparation> compare(IStringifier& stringifier, IBuilding* building, int level) override {
        auto& params = ParamsManager::Instance().params.guns.radar;
        Radar* radar = static_cast<Radar*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier.compare(
            "Уровень маскировки",
            params.uncovering_level_upgrades[level - 1].uncovering_level,
            params.uncovering_level_upgrades[level].uncovering_level
        ));
        return out;
    }

    int cost(int level) override {
        auto& params = ParamsManager::Instance().params.guns.radar;
        return params.uncovering_level_upgrades[level].cost;
    }
};

struct RadarUncoveringSpeedUpgrade : public Upgrade {
    RadarUncoveringSpeedUpgrade() {
        name = "Скорость обнаружения";
        general_description = "Время выбора цели (наведения) и время её обнаружения уменьшиться.";
        auto& params = ParamsManager::Instance().params.guns.radar;
        max_level = params.uncovering_speed_upgrades.size() - 1;
        upgrade_conditions = {};
    }

    void upgrade(IBuilding* building, int goal) override {
        Radar* radar = static_cast<Radar*>(building);
        radar->upgrade_uncovering_speed(goal);
    }

    int get_current_upgrate(IBuilding* building) override {
        Radar* radar = static_cast<Radar*>(building);
        return radar->uncovering_speed_upgrade;
    }

    std::vector<IStringifier::Comparation> compare(IStringifier& stringifier, IBuilding* building, int level) override {
        auto& params = ParamsManager::Instance().params.guns.radar;
        Radar* radar = static_cast<Radar*>(building);
        std::vector<IStringifier::Comparation> out;
        out.push_back(stringifier.compare(
            "Время наведения",
            params.uncovering_speed_upgrades[level - 1].aiming_time,
            params.uncovering_speed_upgrades[level].aiming_time
        ));
        out.push_back(stringifier.compare(
            "Время обнаружения цели",
            params.uncovering_speed_upgrades[level - 1].uncover_time,
            params.uncovering_speed_upgrades[level].uncover_time
        ));
        return out;
    }

    int cost(int level) override {
        auto& params = ParamsManager::Instance().params.guns.radar;
        return params.uncovering_speed_upgrades[level].cost;
    }
};

struct RadarLongDistanceCommunicationUpgrade : public Upgrade {
    RadarLongDistanceCommunicationUpgrade() {
        name = "Дальняя связь";
        general_description = "Радар получит возможность быть частью сети, образованной радиовышками.";
        auto& params = ParamsManager::Instance().params.guns.radar;
        max_level = 1;
        upgrade_conditions = {};
    }

    void upgrade(IBuilding* building, int goal) override {
        Radar* radar = static_cast<Radar*>(building);
        radar->upgrade_long_distance_communication();
    }

    int get_current_upgrate(IBuilding* building) override {
        Radar* radar = static_cast<Radar*>(building);
        return radar->long_distance_communication_upgrade;
    }

    std::vector<IStringifier::Comparation> compare(IStringifier& stringifier, IBuilding* building, int level) override {
        return {};
    }

    int cost(int level) override {
        auto& params = ParamsManager::Instance().params.guns.radar;
        return params.long_distance_communication_upgrade_cost;
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
    RadarRadiusUpgrade radar_radius_upgrade;
    RadarUncoveringLevelUpgrade radar_uncovering_level_upgrade;
    RadarUncoveringSpeedUpgrade radar_uncovering_speed_upgrade;
    RadarLongDistanceCommunicationUpgrade radar_long_distance_communication_upgrade;
    void unlock_all();
private:
    AchievementSystem();
    std::unordered_map<BuildingType, bool> m_unlocked_buildings;
    std::unordered_map<EnemyType, std::vector<BuildingType>> m_building_unlock_achievements;
    std::vector<Upgrade*> m_upgrades;
};
