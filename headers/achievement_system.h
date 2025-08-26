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
    void defeated(EnemyType enemy_type);
    bool is_unlocked(BuildingType type);
    std::string get_achievement_description(BuildingType type);
private:
    AchievementSystem();
    std::unordered_map<EnemyType, BuildingType> m_achievements;
private:
    std::unordered_map<BuildingType, bool> m_unlocked_buildings;

};
