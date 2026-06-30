#pragma once
#include <numeric>
#include <unordered_map>

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

    void enemy_covering_level(uint32_t id, int level);
    void enemy_uncovering_level(uint32_t id, int level);
    bool is_available_taget(uint32_t id);
public:
    struct EnemyStatus {
        int covering_level;
        int uncovering_level;
    };
    EnemyStatus get_status(uint32_t id);
    std::unordered_map<uint32_t, EnemyStatus> m_enemies;
private:
    CoveringDataBase() = default;
private:
    
};
