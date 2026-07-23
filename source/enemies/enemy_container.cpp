#include "enemies/enemy_container.h"
#include "enemies/IEnemy.h"


void EnemyContainer::create(size_t n) {
    assert(n >= 1 && "Cannot create empty");
    m_data.resize(n);
    m_slots.resize(n);
    for (size_t i = 0; i < m_slots.size(); ++i) {
        m_slots[i].data_index = i;
        m_data[i].slot_index = i;
    }
}

IEnemy* EnemyContainer::get_enemy_by_id(EnemyID id) {
    auto& slot = m_slots[id.slot_index];
    if (slot.generation == id.generation)
        return m_data[slot.data_index].enemy.get();
    return nullptr;
}

IEnemy* EnemyContainer::add_enemy(std::unique_ptr<IEnemy>&& enemy) {
    //1. Найдем свободный слот
    assert(m_size < m_data.size() && "Container is full");

    //2. Займем слот, указывающий на ячейку, следующую за последней активной
    Slot* free_slot = &m_slots[m_data[m_size].slot_index];
    ++free_slot->generation;
    //3. Получим данные слота и запишем в них enemy
    auto& data = m_data[free_slot->data_index];
    data.enemy = std::move(enemy);
    data.slot_index = static_cast<size_t>(free_slot - m_slots.data());
    //4. Увеличим размер
    ++m_size;
    //5. Сформируем id и вернем data.
    EnemyContainer::EnemyID id;
    id.generation = free_slot->generation;
    id.slot_index = static_cast<size_t>(free_slot - m_slots.data());
    data.enemy->id = id;
    return data.enemy.get();
}

void EnemyContainer::delete_enemy(EnemyID id) {
    auto& slot = m_slots[id.slot_index];
    if (slot.data_index >= m_size)
        return; //слот пуст.
    if (slot.generation != id.generation)
        return;
    //1. Удаляем данные (и перемещаем их в конец)
    auto& back_data = m_data[m_size - 1];
    auto& data = m_data[slot.data_index];
    data.enemy.reset();
    std::swap(back_data, data); //теперь в m_data[m_size] лежит nullptr
    //2. Корректируем указатели слотов (их данные поменялись местами, значит меняем местами data_index)
    std::swap(m_slots[data.slot_index].data_index, m_slots[back_data.slot_index].data_index);
    //4. уменьшаем размер
    --m_size;
}

EnemyContainer::Iterator EnemyContainer::erase(Iterator& it) {
    delete_enemy((*it)->id);
    return it;
}

bool operator==(EnemyContainer::EnemyID left, EnemyContainer::EnemyID right) {
    return left.generation == right.generation && left.slot_index == right.slot_index;
}


IEnemy* EnemyContainer::Iterator::operator*() const {
    assert(m_current && "Attempt to dereference null pointer");
    return m_current->enemy.get();
}


EnemyContainer::Iterator& EnemyContainer::Iterator::operator++() {
    ++m_current;
    return *this;
}

