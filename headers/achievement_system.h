#include "tile_map.h"
#include "enemies/IEnemy.h"
#include <functional>
#include <unordered_map>

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
    std::string get_achievement_description(BuildingType type);

    struct MinigunUpgrades {
        int penetration_upgrade = 0;
        int cooling_upgrade = 0;
        int lubricant_update = 0;
    } minigun_upgrades;

private:
    AchievementSystem();
    std::unordered_map<EnemyType, BuildingType> m_achievements;
private:
    std::unordered_map<BuildingType, bool> m_unlocked_buildings;
    std::map<EnemyType, std::function<void()>> m_general_achievements;
};
