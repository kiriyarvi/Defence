#include <covering_database.h>
#include <iostream>

/// делает врага скрытым. Если врага нет в базе, добавляет его.
void CoveringDataBase::make_enemy_covered(uint32_t id) {
    auto it = m_enemies.find(id);
    if (it == m_enemies.end())
        m_enemies.emplace(id, EnemyStatus::Covered);
    else
        it->second = EnemyStatus::Covered;
    std::cout << "Covered " << id << std::endl;
}

/// делает врага рассекреченным. Если врага нет, ничего не добавляет
void CoveringDataBase::make_enemy_uncovered(uint32_t id) {
    auto it = m_enemies.find(id);
    if (it != m_enemies.end())
        it->second = EnemyStatus::Uncovered;
    std::cout << "Enemy " << id <<" uncovered"  << std::endl;
}

/// Удаляет врага
void CoveringDataBase::remove_enemy(uint32_t id) {
    std::cout << "Removes " << id << std::endl;
    m_enemies.erase(id);
}

// цель допустима для стрельбы по ней, если она либо не находится в базе, либо раскрыта
bool CoveringDataBase::is_available_taget(uint32_t id) {
    auto it = m_enemies.find(id);
    return it == m_enemies.end() ? true : (it->second == EnemyStatus::Uncovered);
}

CoveringDataBase::EnemyStatus CoveringDataBase::get_status(uint32_t id) {
    auto it = m_enemies.find(id);
    return it == m_enemies.end() ? EnemyStatus::Unknown : it->second;
}
