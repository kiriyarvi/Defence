#pragma once
#include <memory>
#include <vector>
#include <functional>

class IEnemy;

/**
 * @brief Контейнер для хранения IEnemy
 * @details Добавление O(1), удаление O(1), перебор соотвествует числу добавленых элементов.
 * IEnemy хранятся в непрерывном блоке памяти, так что при проходе по нему не нужно совершать
 * слишком больших прыжков по памяти. Работает так. Если массив данных m_data
 * и массив слотов m_slots. Между слотами и данными установлено взаимооднозначеное соотвествие.
 * если m_size. Элементы m_data с индексами меньшими m_size считаются свободными.
 * при добавлении нового врага берется слот, указывающий на первый свободный элемент m_data и именно по нему
 * и заполняется m_data (то есть генерируется generation по этому slot)
 * Удаление, для сохранения плотности m_data реализуется с помощью std::swap с последний несвободным элементом
 * и пересчета указателей.
 */
class EnemyContainer {
private:
    struct Slot {
        size_t generation = 0; //поколоение объекта
        size_t data_index = 0; //индекс в m_data
    };
    struct Data {
        std::unique_ptr<IEnemy> enemy;
        size_t slot_index = 0;
    };
    std::vector<Data> m_data;
    std::vector<Slot> m_slots;
    size_t m_size = 0; //< количество врагов (или активных слотов)
public:
    void create(size_t n);

    struct EnemyID {
        size_t generation; //< поколоение объекта
        size_t slot_index; //< интекс слота
    };

    IEnemy* get_enemy_by_id(EnemyID id);
    IEnemy* add_enemy(std::unique_ptr<IEnemy>&& enemy);
    void delete_enemy(EnemyID id);



    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = IEnemy*;
        using difference_type = std::ptrdiff_t;
        using pointer = IEnemy**;
        using reference = IEnemy*;
        Iterator(Data* current) : m_current(current) {}
        IEnemy* operator*() const;
        Iterator& operator++();

        bool operator==(const Iterator& other) const {  return m_current == other.m_current; }
        bool operator!=(const Iterator& other) const {  return m_current != other.m_current; }
    private:
        Data* m_current;
    };
    Iterator erase(Iterator& it);

    Iterator begin() { return Iterator(m_data.data()); }
    Iterator end() {  return Iterator(m_data.data() + m_size); }
    size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }
};

bool operator==(EnemyContainer::EnemyID left, EnemyContainer::EnemyID right);

template<>
struct std::hash<EnemyContainer::EnemyID> {
    std::size_t operator()(
        const EnemyContainer::EnemyID& id
        ) const noexcept {
        std::size_t h1 =
            std::hash<std::size_t>{}(id.generation);

        std::size_t h2 =
            std::hash<std::size_t>{}(id.slot_index);

        return h1 ^ (h2 + 0x9e3779b9 +
            (h1 << 6) + (h1 >> 2));
    }
};
