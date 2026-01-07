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

    void make_enemy_covered(uint32_t id);
    void make_enemy_uncovered(uint32_t id);
    void remove_enemy(uint32_t id);
    bool is_available_taget(uint32_t id);
public:
    enum class EnemyStatus {
        Covered,
        Uncovered,
        Unknown
    };
    EnemyStatus get_status(uint32_t id);
    std::unordered_map<uint32_t, EnemyStatus> m_enemies;
private:
    CoveringDataBase() = default;
private:
    
};
