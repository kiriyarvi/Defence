#include "tile_map.h"
#include "enemies/IEnemy.h"
#include <functional>
#include <unordered_map>

struct ICondition {
    virtual std::string get_description() = 0;
    virtual bool defeated(EnemyType enemy_type) = 0;
};

using Condition = std::unique_ptr<ICondition>;

class EnemyDefeatedCondition: public ICondition {
public:
    EnemyDefeatedCondition(EnemyType type);
    std::string get_description() override;
    bool defeated(EnemyType enemy_type)override;
private:
    EnemyType m_type;
};

struct Upgrade {
    Upgrade(const std::string& name, BuildingType building_type, int* upgrade_variable, int goal_value):
        name{ name }, upgrade_variable{ upgrade_variable }, goal_value{ goal_value } {}
    std::string name;
    BuildingType building_type;
    int* upgrade_variable;
    int goal_value;
};

struct Achievement {
    Condition condition;
    std::vector<BuildingType> unlocked_buildings;
    std::vector<Upgrade> upgrades;
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
    std::string get_upgrade_unlock_condition_description(int* var, int goal_value);

    static struct MinigunUpgrades {
        int penetration_upgrade = 0;
        int cooling_upgrade = 0;
        int lubricant_update = 0;
        static Upgrade create_penetration_upgrade(int n);
        static Upgrade create_lubricant_upgrade(int n);
        static Upgrade create_cooling_upgrade(int n);
    } minigun_upgrades;


    void unlock_all();
private:
    AchievementSystem();
    std::list<Achievement> m_achievements;
private:
    std::unordered_map<BuildingType, bool> m_unlocked_buildings;
};
