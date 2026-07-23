#include <covering_database.h>
#include <iostream>

/// Устанавливает статус скрытости врага (полностью переписывает covering_level)
/// Удаляет врага из базы данных, если level == 0.
void CoveringDataBase::enemy_covering_level(EnemyContainer::EnemyID id, int level) {
    if (level == 0) { //удаляем из базы данных
        m_enemies.erase(id);
    }
    else { //добавим или уточним информацию
        auto it = m_enemies.find(id);
        if (it == m_enemies.end())
            m_enemies.emplace(id, EnemyStatus{ level, 0 });
        else
            it->second.covering_level = level;
    }
}

/// Устанавливает статус раскрытости врага. Статус не перезаписывается, а выбирается максимальное значение из предоставленного и того, что уже есть.
void CoveringDataBase::enemy_uncovering_level(EnemyContainer::EnemyID id, int level) {
    auto it = m_enemies.find(id);
    if (it != m_enemies.end())
        it->second.uncovering_level = std::max(level, it->second.uncovering_level);
}

// цель допустима для стрельбы по ней, если она либо не находится в базе, либо раскрыта
bool CoveringDataBase::is_available_taget(EnemyContainer::EnemyID id) {
    auto it = m_enemies.find(id);
    return it == m_enemies.end() ? true : (it->second.uncovering_level >= it->second.covering_level);
}

CoveringDataBase::EnemyStatus CoveringDataBase::get_status(EnemyContainer::EnemyID id) {
    auto it = m_enemies.find(id);
    return it == m_enemies.end() ? EnemyStatus{0,0} : it->second;
}
