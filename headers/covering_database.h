#pragma once
#include <numeric>
#include <unordered_map>
#include <enemies/enemy_container.h>

class CoveringDataBase {
public:
    static CoveringDataBase& Instance() {
        static CoveringDataBase instance;
        return instance;
    }

    // Удаляем копирование и перемещение
    CoveringDataBase(const CoveringDataBase&) = delete;
    CoveringDataBase& operator=(const CoveringDataBase&) = delete;
    CoveringDataBase(CoveringDataBase&&) = delete;
    CoveringDataBase& operator=(CoveringDataBase&&) = delete;

    void enemy_covering_level(EnemyContainer::EnemyID id, int level);
    void enemy_uncovering_level(EnemyContainer::EnemyID, int level);
    bool is_available_taget(EnemyContainer::EnemyID);
public:
    struct EnemyStatus {
        int covering_level;
        int uncovering_level;
    };
    EnemyStatus get_status(EnemyContainer::EnemyID);
    std::unordered_map<EnemyContainer::EnemyID, EnemyStatus> m_enemies;
private:
    CoveringDataBase() = default;
private:
    
};
