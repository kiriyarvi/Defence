#include "gui/widget.h"
#include <numeric>
#include <iostream>
#include <optional>

#ifdef GUI_USER_CONTRACT_CHECKS_ENABLED
    #include <unordered_set>
#endif

Query Query::skip(bool from_subscribe){
    if (from_subscribe)
        return Query{ Query::PASS };
    else
        return Query{ Query::PASS_TO_PARENT };
}

void GUI::set_root(std::unique_ptr<Widget>&& root, const sf::RenderWindow& window) {
    m_root = std::move(root);
    window_size = { window.getSize().x ,  window.getSize().y };
    m_root->add_rule(Property::SIZE, [this](Widget::Layout& layout) {
        layout.width = window_size.x;
        layout.height = window_size.y;
    }, {});
}

void GUI::draw(sf::RenderWindow& window) {
    if (m_frame == std::numeric_limits<size_t>::max()) {
        m_frame = 0;
    }
    ++m_frame;
    if (m_root) {
        auto view = window.getView();
        auto w_size = window.getSize();
        window.setView(sf::View(sf::FloatRect(0, 0, w_size.x, w_size.y)));
        m_root->draw_hierarchy(m_frame, { 0,0 }, window);
        window.setView(view);
    }
#ifdef GUI_DEBUG_ENABLED
    //std::cout << "Done draw call\n\n";
#endif
}

class WidgetIterator {
public:
    void init(const std::list<Widget::HitListNode>* hit_test_list, const std::list<GUI::Subscriber>* subscribers, Event::Type event_type) {
        m_hit_test_list = hit_test_list;
        m_subscribers = subscribers;
        m_event_type = event_type;

        m_subs_iterator = m_subs_iterator = std::find_if(m_subscribers->begin(), m_subscribers->end(), [et = m_event_type](const GUI::Subscriber& subscriber) {
            return subscriber.event_type & et;
        });;
        m_hit_test_iterator = m_hit_test_list->empty() ? m_hit_test_list->end() : std::prev(m_hit_test_list->end());

        m_subscriber = (m_subs_iterator != m_subscribers->end());
        if (m_subscriber)
            m_current = m_subs_iterator->subscriber;
        else if (m_hit_test_iterator != m_hit_test_list->end())
            m_current = m_hit_test_iterator->widget;
        else
            m_current = nullptr;
    }
    Widget* get_current() const { return m_current; }
    void next() {
        if (m_subscriber) {
            m_subs_iterator = std::find_if(std::next(m_subs_iterator), m_subscribers->end(), [et = m_event_type](const GUI::Subscriber& subscriber) {
                return subscriber.event_type & et;
            });
            if (m_subs_iterator != m_subscribers->end()) {
                m_current = m_subs_iterator->subscriber;
            }
            else if (!m_hit_test_list->empty()) {
                m_current = m_hit_test_list->back().widget;
                m_hit_test_iterator = m_hit_test_list->empty() ? m_hit_test_list->end() : std::prev(m_hit_test_list->end());
                m_subscriber = false;
            }
            else {
                m_subscriber = false;
                m_current = nullptr;
            }
        }
        else { 
            m_subscriber = false;
            m_current = nullptr;
        }
    }
    void to_parent() {
        assert(!m_subscriber && "Invalid operation!");
        assert(m_current && "Invalid operation!");
        if (m_hit_test_iterator == m_hit_test_list->begin()) {
            m_current = nullptr;
        }
        else {
            m_hit_test_iterator = std::prev(m_hit_test_iterator);
            m_current = m_hit_test_iterator->widget;
        }
    }
    bool is_subscriber() { return m_subscriber; }
private:
    const std::list<Widget::HitListNode>* m_hit_test_list;
    const std::list<GUI::Subscriber>* m_subscribers;
    std::list<GUI::Subscriber>::const_iterator m_subs_iterator;
    std::list<Widget::HitListNode>::const_iterator m_hit_test_iterator;
    Widget* m_current = nullptr;
    bool m_subscriber = true;
    Event::Type m_event_type;
};

/// true - событие обработано
bool GUI::event(const sf::Event& event) {
    m_event_processing = true;
    if (!m_root)
        return false;

    if (event.type == sf::Event::Resized) {
        window_size = { event.size.width , event.size.height };
        m_root->invalidate(Property::SIZE);
        return false; //дать возможность обработать событие камере
    }

    //0. Мы обрабадываем только определенные типы событий
    bool valid_event =
        (event.type == sf::Event::MouseMoved ||
            event.type == sf::Event::MouseButtonPressed ||
            event.type == sf::Event::MouseButtonReleased ||
            event.type == sf::Event::MouseWheelScrolled);
    if (!valid_event)
        return false;
    //1. Заполним переменные контекста
    if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased) {
        mouse_pos = { event.mouseButton.x, event.mouseButton.y };
        mouse_button = event.mouseButton.button;
        event_type = event.type == sf::Event::MouseButtonPressed ? Event::BUTTON_PRESSED : Event::BUTTON_RELEASED;
    }
    else if (event.type == sf::Event::MouseMoved) {
        mouse_pos = { event.mouseMove.x, event.mouseMove.y };
        event_type = Event::MOUSE_MOVED;
    }
    else if (event.type == sf::Event::MouseWheelScrolled) {
        mouse_pos = { event.mouseWheelScroll.x, event.mouseWheelScroll.y };
        wheel_delta = event.mouseWheelScroll.delta;
        event_type = Event::WHEEL_SCROLLED;
    }
    //2. Обработаем событие
    bool processed = event_impl();
    perform_deffered();
    m_event_processing = false;
    return processed;
}

bool GUI::event_impl() {
    while (true) { //REPEAT-while
        //1. hit-test
        std::list<Widget::HitListNode> hit_test;
        m_root->hit_test(hit_test, mouse_pos);
#ifdef GUI_DEBUG_ENABLED
        for (auto& w : hit_test) {
            std::cout << w.widget->debug_name << " -> ";
        }
        if (!hit_test.empty())
            std::cout << std::endl;
#endif
        //2. получаем виджет для обработки события
        WidgetIterator widget_iterator;
        widget_iterator.init(&hit_test, &m_subscribers, event_type);
        while (true) { //WidgetIterator-while
            Widget* target = widget_iterator.get_current();
            if (target == nullptr)
                return false; //некому обработать событие
            bool subscriber = widget_iterator.is_subscriber();
            Widget::EventContext context{ hit_test, event_type, widget_iterator.is_subscriber() };
            //3. отправляем виджету событие
            auto query = target->on_event(context);
            //4. валидация возвращаемого значения
            if (subscriber) {
                assert(query.workflow != Query::Workflow::PASS_TO_PARENT);
                if (query.query & Query::PERFORM_DEFFERED)
                    assert(query.workflow == Query::Workflow::REPEAT && "Invalid workflow");
            }
            //5. Выполнение запросов
            if (query.query & Query::PERFORM_DEFFERED) {
                assert(query.workflow == Query::Workflow::REPEAT && "Invalid workflow");
                perform_deffered();
            }
            if (query.query & Query::CALC_LAYOUT) {
                m_root->calc_layout();
            }
            //6. Workflow
            if (query.workflow == Query::Workflow::PASS) {
                widget_iterator.next();
            }
            else if (query.workflow == Query::Workflow::PROCESSED)
                return true; //событие обработано
            else if (query.workflow == Query::Workflow::REPEAT) { //повторить обработку событий
                hit_test.clear();
                break; // нужно вернуться к внешнему while
            }
            else if (query.workflow == Query::Workflow::PASS_TO_PARENT) {
                widget_iterator.to_parent();
            }
        }
    }
}

void GUI::subscribe_deffered(Widget* widget, Event::Type type) {
    m_deffered_commands.push_back([this, widget, type]() {
        auto sub = std::find_if(m_subscribers.begin(), m_subscribers.end(), [widget](Subscriber& subscriber) {
            return subscriber.subscriber == widget;
        });
        if (sub != m_subscribers.end())
            sub->event_type |= type;
        else
            m_subscribers.push_back({ type, widget });
    });
}

void GUI::unsubscribe_deffered(Widget* widget, Event::Type type) {
    m_deffered_commands.push_back([this, widget, type]() {
        auto sub = std::find_if(m_subscribers.begin(), m_subscribers.end(), [widget](Subscriber& subscriber) {
            return subscriber.subscriber == widget;
        });
        if (sub == m_subscribers.end())
            return;
        sub->event_type &= ~type;
        if (sub->event_type == 0)
            m_subscribers.erase(sub);
    });
}

void GUI::perform_deffered() {
    for (auto it = m_deffered_commands.begin(); it != m_deffered_commands.end(); ++it)
        (*it)();
    m_deffered_commands.clear();
}

Property::Type Property::X =       0b0001;
Property::Type Property::Y =       0b0010;
Property::Type Property::WIDTH =   0b0100;
Property::Type Property::HEIGHT =  0b1000;
Property::Type Property::SIZE =    0b1100;
Property::Type Property::POSITION =0b0011;
Property::Type Property::LAYOUT =  0b1111;

size_t Anchor::LEFT = 0b1000;
size_t Anchor::RIGHT = 0b0100;
size_t Anchor::TOP = 0b0010;
size_t Anchor::BOTTOM = 0b0001;
size_t Anchor::CENTER = 0;

const std::unordered_map<Property::Type, Property Widget::Layout::*> Widget::Layout::s_property_map {
    {Property::X, &Widget::Layout::x},
    {Property::Y, &Widget::Layout::y},
    {Property::WIDTH, &Widget::Layout::width},
    {Property::HEIGHT, &Widget::Layout::height}
};

const std::unordered_map<Property::Type, float Rect::*> Widget::Layout::s_property_map_rect{
    {Property::X, &Rect::x},
    {Property::Y, &Rect::y},
    {Property::WIDTH, &Rect::width},
    {Property::HEIGHT, &Rect::height}
};


/// Возвращает прямоугольник контента
Rect Widget::Layout::get_content_rect() const {
    Rect r;
    r.x = x + padding.left;
    r.y = y + padding.top;
    r.height = height - padding.top - padding.bottom;
    r.width = width - padding.left - padding.right;
    return r;
}

/// Возвращает прямоугольник виджета
Rect Widget::Layout::get_layout_rect() const {
    Rect r;
    r.x = x;
    r.y = y;
    r.height = height;
    r.width = width;
    return r;
}

/// Возвращает, какой размер должен быть у виджета, чтобы вместить контент с размером content_size
glm::vec2 Widget::Layout::layout_size(const glm::vec2& content_size) const {
    return {
        content_size.x + padding.left + padding.right,
        content_size.y + padding.top + padding.bottom
    };
}

/// Возвращает координаты указанного якоря относительно центра виджета
glm::vec2 Widget::Layout::get_anchor_relative_to_center(Anchor::Type anchor) const {
    glm::vec2 r = { width / 2.f , height / 2.f };
    glm::vec2 center = { 0,0 };
    if (anchor & Anchor::BOTTOM)
        center.y += r.y;
    if (anchor & Anchor::TOP)
        center.y -= r.y;
    if (anchor & Anchor::LEFT)
        center.x -= r.x;
    if (anchor & Anchor::RIGHT)
        center.x += r.x;
    return center;
}

/// Возвращает координаты центра виджета
glm::vec2 Widget::Layout::center() const {
    return { x + width / 2.f, y + height / 2.f };
}

/// Содержит ли виджет данную точку?
bool Widget::Layout::contains(const glm::vec2& transform, float px, float py) {
    px -= transform.x;
    py -= transform.y;
    return px >= x && px <= x + width && py >= y && py <= y + height;
}


void Property::add_depentent(Dependent dependent) {
    auto it = std::find_if(dependents.begin(), dependents.end(), [widget = dependent.widget](const Dependent& d) { return widget == d.widget; });
    if (it == dependents.end())
        dependents.push_back(dependent);
    else {
        assert((it->output & dependent.output) == 0 || "Invalid operations"); //иначе некорректно удаление
        it->output |= dependent.output;
    }
}

void Property::clear_dependent(Dependent dependent) {
    auto it = std::find_if(dependents.begin(), dependents.end(), [widget = dependent.widget](const Dependent& d) { return widget == d.widget; });
    if (it == dependents.end())
        return;
    it->output &= ~dependent.output;
    if (it->output == 0)
        dependents.erase(it);
}

/// Удаляет правила вычисления свойств properties. Инвалидирует properties
void Widget::clear_rules(Property::Type properties) {
    for (auto rule = m_rules.begin(); rule != m_rules.end();) {
        if ((rule->output & properties) == rule->output) { //rule->properties всключает все флаги что и properties
            for (auto& dependency : rule->dependencies) {
                //должны сказать зависимостям, что мы больше вычисление свойств rule->properties данного виджета не зависит от них.
                for (auto& [prop, member] : Layout::s_property_map) {
                    if (dependency.source & prop)
                        (dependency.widget->layout.*member).clear_dependent({ this, rule->output });
                }
            }
            rule = m_rules.erase(rule);
            continue;
        }
        assert((rule->output & properties) == 0 && "Invalid operation"); //правило должно быть перпендикулярно properties.
        //если правило вычисляет X,Y, то мы не можем попросить удалить только X.
        ++rule;
    }
    invalidate(properties);
}


/**
 * @brief Добавляет новое правило вычисления.
 * @param properties Свойства, для которых будет применяться данное правило
 * @param calc функция, вычисляющая данные свойства
 * @param dependencies зависимости, т.е. виджеты и их свойства, которые должны быть вычислены на момент выполнения calc.
 * @details удаляет правила, которые ранее вычисляли свойства properties.
 * инвалидирует свойства output.
 * @note массив dependencies не должет содержать повторяющихся виджетов.
 */
void Widget::add_rule(Property::Type output, const std::function<void(Layout&)>& calc, const std::vector<Dependency>& dependencies) {
#ifdef GUI_USER_CONTRACT_CHECKS_ENABLED
    std::unordered_set<Widget*> set;
    for (const auto& item : dependencies)
        assert(set.insert(item.widget).second && "Duplicates in dependencies list");
#endif
    clear_rules(output);
    m_rules.push_back({ output, calc, dependencies });
    for (auto& dependency : dependencies) {
        for (auto& [prop, member] : Layout::s_property_map) {
            if (dependency.source & prop)
                (dependency.widget->layout.*member).add_depentent({ this, output });
        }
    }
}

/// Вычисляет свойства виджета (свойства - это x,y,width, height), указанные в битовой маске property.
void Widget::calc_properties(Property::Type property) {
#ifdef GUI_USER_CONTRACT_CHECKS_ENABLED
    auto& stack = GUI::Instance().layout_stack;
    assert(std::find(stack.begin(), stack.end(), GUI::StackElement{ this, property }) == stack.end());
    stack.push_back(GUI::StackElement{ this, property });
#endif
    auto current_frame = GUI::Instance().get_current_frame_id();
    //не будем вычислять то, что не было инвалидировано и что уже было вычислено на этом этапе.
    Property::Type invalidated_props = layout.invalidated_props();
    for (auto& [prop, member] : Layout::s_property_map) {
        if (!(invalidated_props & prop) || (layout.*member).last_calculation_frame_id == current_frame)
            property &= ~prop;
    }
    if (property == 0) {
#ifdef GUI_USER_CONTRACT_CHECKS_ENABLED
        //std::cout << "properties have been calculated." << std::endl;
        stack.pop_back();
#endif
        return;
    }

#ifdef GUI_DEBUG_ENABLED
    std::cout << "calc " << current_frame << " [" <<
        ((property & Property::X) ? "X," : "") <<
        ((property & Property::Y) ? "Y," : "") <<
        ((property & Property::WIDTH) ? "WIDTH," : "") <<
        ((property & Property::HEIGHT) ? "HEIGHT," : "") << "] for " << debug_name << std::endl;
#endif
    //выполним все правила, которые вычисляют properties пересекающиеся с property.
    for (auto& rule : m_rules) {
        if ((rule.output & property) != 0) { //выполняем все правила, которые вычисляют набор свойств, пересекающийся с property.
            for (auto& dependency : rule.dependencies) { //вычисляем зависимости
                dependency.widget->calc_properties(dependency.source);
            }  
            rule.calc_function(layout); //теперь вычислим правило
            layout.m_invalidated_props &= ~rule.output; //и пометим вычисленные свойства валидными.
            //обновляем frame для вычисленных полей
            for (auto& [prop, member] : Layout::s_property_map) {
                if (prop & rule.output)
                    (layout.*member).last_calculation_frame_id = current_frame;
            }
        }
    }
    //Experimental (use integer coords to prevent artefacts)
    for (auto& [prop, member] : Layout::s_property_map) {
        if (prop & property)
            (layout.*member) = std::round((layout.*member));
    }
    //для некоторых свойств не нашлось правил для их вычисления => применяем правила по умолчанию
    Property::Type required = layout.m_invalidated_props & property;
    if (required != 0) {
        if (required & Property::X) {
            layout.x = 0;
            layout.x.last_calculation_frame_id = current_frame;
        }
        if (required & Property::Y) {
            layout.y = 0;
            layout.y.last_calculation_frame_id = current_frame;
        }
        assert((required & Property::WIDTH)== 0 && "no rule for WIDTH");
        assert((required & Property::HEIGHT) == 0 && "no rule for HEIGHT");
        layout.m_invalidated_props &= ~required; //validate
    }

#ifdef GUI_USER_CONTRACT_CHECKS_ENABLED
    stack.pop_back();
#endif
}

/// Вычисляет весь Layout виджета. 
void Widget::calc_layout() {
    calc_properties(Property::LAYOUT);
}

void Widget::invalidate(Property::Type property) {
    if ((layout.m_invalidated_props & property) == property)
        return; //уже инвалидировано
    Property::Type old_invalidated = layout.m_invalidated_props;
    layout.invalidate(property); //инвалидируем
    for (auto& [prop, member] : Layout::s_property_map) {
        if (prop & property & (~old_invalidated)) { //свойство пересекается с теми, которые просят инвализировать и пересекается с теми, которые еще не инвалидированы
            for (auto& dependent : (layout.*member).dependents) {
                //у зависимого нужно инвалидировать те свойства, которые вычисляются по member. Они как раз указаны в dependent.output.
                dependent.widget->invalidate(dependent.output);
            }
        }
    }
}

glm::vec2 Widget::get_position_transform() const {
    if (m_parent == nullptr)
        return glm::vec2{ 0,0 };
    auto parent_content_transform = m_parent->get_content_transform();
    return parent_content_transform + glm::vec2{ layout.x,  layout.y };
}

glm::vec2 Widget::get_content_transform() const {
    if (m_parent == nullptr)
        return glm::vec2{ layout.padding.left, layout.padding.top };
    auto parent_content_transform = m_parent->get_content_transform();
    return parent_content_transform + glm::vec2{ layout.x + layout.padding.left,  layout.y + layout.padding.top };
}

/**
 * @brief Общая функция для создения правила вычисления свойств виджета по свойствам виджета source
 * @param output Какое свойство будет вычисляться
 * @param content_output Будет вычисляться свойство прямоугольника контента данного виджета, а затем уже по нему автоматически вычислиться layout
 * @param source Виджет-источник
 * @param input Свойство source по которому будем вычислять
 * @param content_input брать input из прямоугольника контента?
 * @param modifier модификатор. Применяется к input (c учетом content_input).
 * @note Добавляемое правило
 * OUTPUT: output
 * INPUT: input from source.
 */
void Widget::property_equal(Property::Type output, bool content_output, Widget* source, Property::Type input, bool content_input, const Modifier& modifier) {
    assert((output == Property::X || output == Property::Y || output == Property::WIDTH || output == Property::HEIGHT) && "Invalid value!");
    assert((input == Property::X || input == Property::Y || input == Property::WIDTH || input == Property::HEIGHT) && "Invalid value!");
    auto [_, output_member] = *Layout::s_property_map.find(output);
    auto [__, input_member] = *Layout::s_property_map.find(input);
    auto [___, input_content_member] = *Layout::s_property_map_rect.find(input);
    add_rule(output, [=](Layout& layout) {
        float input_value = content_input ? (source->layout.get_content_rect().*input_content_member) : (source->layout.*input_member);
        if (modifier)
            input_value = modifier(input_value);
        if (!content_output) {
            layout.*output_member = input_value;
        }
        else {
            if (output == Property::WIDTH)
                layout.*output_member = input_value + layout.padding.left + layout.padding.right;
            else if (output == Property::HEIGHT)
                layout.*output_member = input_value + layout.padding.top + layout.padding.bottom;
            else if (output == Property::X)
                layout.*output_member = input_value - layout.padding.left; //поскольку input_value это x-овая координата контента
            else// (output == Property::Y)
                layout.*output_member = input_value - layout.padding.top;
        }
    }, { {source, input} });
}


/**
 * @brief Свойства properties устанавливаются равными свойствам прямоугольника контента source.
 * Дополнительно может быть применен modifier. Между свойствами устанавливается тождественное соотвествие,
 * то есть this.x вычисляется по widget.x, this.y по widget.y и так далее.
 * В зависимости от значения properties может быть добавлено до четырех правил.
 */
void Widget::property_from_content(Widget* source, Property::Type properties, const Modifier& modifier) {
    for (auto [prop, _] : Layout::s_property_map)
        if (prop & properties)
            property_equal(prop, false, source, prop, true, modifier);
}

/**
 * @brief Свойства properties прямоугольника контента устанавливаются равными свойствам source.
 * Дополнительно может быть применен modifier. Между свойствами устанавливается тождественное соотвествие,
 * то есть this.x вычисляется по widget.x, this.y по widget.y и так далее.
 * В зависимости от значения properties может быть добавлено до четырех правил.
 */
void Widget::property_content_from(Widget* source, Property::Type properties, const Modifier& modifier) {
    for (auto [prop, _] : Layout::s_property_map)
        if (prop & properties)
            property_equal(prop, true, source, prop, false, modifier);
}

/// Устанавливает фиксированный размер
void Widget::size_fixed(float width, float height) {
    add_rule(Property::SIZE, [width, height](Layout& properties) {
        properties.height = height;
        properties.width = width;
    }, {});
}

///short-cut для property_from_content(source, Property::SIZE)
void Widget::size_inherited(Widget* source) {
    property_from_content(source, Property::SIZE);
}

///short-cut для property_content_from(source, Property::SIZE)
void Widget::size_include(Widget* source) {
    property_content_from(source, Property::SIZE);
}

///width и height данного виджета составляют parent_width_fraction, parent_height_fraction от размеров widget.
void Widget::size_fraction(Widget* source, float parent_width_fraction, float parent_height_fraction) {
    add_rule(Property::SIZE, [source, parent_width_fraction, parent_height_fraction](Layout& properties) {
        auto content = source->layout.get_content_rect();
        properties.width = content.width * parent_width_fraction;
        properties.height = content.height * parent_height_fraction;
    }, { {source, Property::SIZE} });
}

/// центрирует виджет внутри parent. parent может быть любым виджетом (по умолчанию всегда родитель), но
/// в случае, когда виджет не родитель, результат может быть не предсказумым, поскольку смещение при отрисовке
/// берется все равно относительно настоящего родителя.
void Widget::position_centering(Widget* parent) {
    if (parent == nullptr)
        parent = m_parent;
    assert(parent != nullptr && "Invalid call");
    add_rule(Property::POSITION, [parent](Layout& layout) {
        layout.x = (parent->layout.width - layout.width) / 2.f;
        layout.y = (parent->layout.height - layout.height) / 2.f;
    }, { {parent, Property::SIZE}, {this, Property::SIZE} });
}

/// Вычисляет позицию виджета так, как будто это tooltip
/// виджет цепляется к позиции мыши за ancher.
/// Вычисление позиции становится абсолютным.
/// Зависимости:
///     SIZE корневого виджета.
void Widget::position_tooltip(size_t ancher) {
    layout.absolute = true;
    add_rule(Property::POSITION, [ancher](Layout& layout) {
        auto position = GUI::Instance().mouse_pos - layout.get_anchor_relative_to_center(ancher) - glm::vec2{layout.width/2.f, layout.height/2.f};
        auto window_size = GUI::Instance().window_size;
        if (position.x + layout.width > window_size.x)
            position.x = window_size.x - layout.width;
        if (position.x < 0)
            position.x = 0;
        if (position.y + layout.height > window_size.y)
            position.y = window_size.y - layout.height;
        if (position.y < 0)
            position.y = 0;
        layout.x = position.x;
        layout.y = position.y;
    }, { {GUI::Instance().get_root(), Property::SIZE} });
}



void Widget::position_anchor(Anchor::Type pivot, Widget* to, Anchor::Type anchor) {
    assert((this->m_parent == to->m_parent || this->m_parent == to) && "position_anchor invalid operation");
    //TODO проверка родителя
    add_rule(Property::POSITION, [pivot, to, anchor](Layout& layout) {
        glm::vec2 anchor_pos = to->layout.get_anchor_relative_to_center(anchor) + to->layout.center();
        glm::vec2 pivot_anchor = layout.get_anchor_relative_to_center(pivot);
        glm::vec2 pos = anchor_pos - pivot_anchor - glm::vec2{ layout.width / 2.f, layout.height / 2.f };
        layout.x = pos.x;
        layout.y = pos.y;
    }, { {to, Property::LAYOUT}, {this, Property::SIZE} });
}

void Widget::vbox(const std::vector<Widget*>& elements) {
    for (size_t i = 1; i < elements.size(); ++i) {
        elements[i]->add_rule(Property::POSITION, [prev = elements[i - 1]](Layout& layout) {
            layout.x = prev->layout.x;
            layout.y = prev->layout.y + prev->layout.height;
        }, { { elements[i - 1], Property::POSITION | Property::HEIGHT } });
    }

    std::vector<Dependency> height_dependencies(elements.size());
    std::vector<Dependency> width_dependencies(elements.size());
    for (size_t i = 0; i < elements.size(); ++i) {
        height_dependencies[i] = { elements[i], Property::HEIGHT };
        width_dependencies[i] = { elements[i], Property::WIDTH };
    }

    add_rule(Property::HEIGHT, [elements](Layout& layout) {
        float height = 0.0;
        for (auto& e : elements) height += e->layout.height;
        layout.height = height;
    }, height_dependencies);

    add_rule(Property::WIDTH, [elements](Layout& layout) {
        float width = 0.0;
        for (auto& e : elements) width = std::max(width, static_cast<float>(e->layout.width));
        layout.width = width;
    }, width_dependencies);
}

void Widget::hbox(const std::vector<Widget*>& elements) {
    for (size_t i = 1; i < elements.size(); ++i) {
        elements[i]->add_rule(Property::POSITION, [prev = elements[i - 1]](Layout& layout) {
            layout.x = prev->layout.x + prev->layout.width;
            layout.y = prev->layout.y;
        }, { { elements[i - 1], Property::POSITION | Property::WIDTH } });
    }

    std::vector<Dependency> height_dependencies(elements.size());
    std::vector<Dependency> width_dependencies(elements.size());
    for (size_t i = 0; i < elements.size(); ++i) {
        height_dependencies[i] = { elements[i], Property::HEIGHT };
        width_dependencies[i] = { elements[i], Property::WIDTH };
    }

    add_rule(Property::HEIGHT, [elements](Layout& layout) {
        float height = 0.0;
        for (auto& e : elements) height = std::max(height, static_cast<float>(e->layout.height));
        layout.height = height;
    }, height_dependencies);

    add_rule(Property::WIDTH, [elements](Layout& layout) {
        float width = 0.0;
        for (auto& e : elements) width += e->layout.width;
        layout.width = width;
    }, width_dependencies);
}


std::unique_ptr<Widget> Widget::create(Widget* parent) {
    return std::make_unique<Widget>(parent);
}

void Widget::draw_hierarchy(int frame, const glm::vec2& position_transform, sf::RenderTarget& window) {
    calc_layout();
    draw(position_transform, window);
    glm::vec2 transform = position_transform + glm::vec2{layout.x + layout.padding.left, layout.y + layout.padding.top};
    for (auto& child : m_children) {
        child->draw_hierarchy(frame, child->layout.absolute ? glm::vec2(0.f, 0.f) : transform, window);
    }
}

bool Widget::hit_test(std::list<Widget::HitListNode>& hit_list, glm::uvec2 mouse_pos) {
    if (hit_test_policy == HitTestPolicy::Terminate)
        return false;
    glm::vec2 parent_transform = (hit_list.empty() || layout.absolute) ? glm::vec2(0, 0) : hit_list.back().parent_transform;
    glm::vec2 content_transform = parent_transform + glm::vec2{ layout.x + layout.padding.left, layout.y + layout.padding.top };
    std::optional<bool> hit_test_passed; //прошел ли this hit-test
    if (hit_test_policy == HitTestPolicy::Block) { //прерываем hit-test, если this не проходит hit-test
        hit_test_passed = layout.contains(parent_transform, mouse_pos.x, mouse_pos.y);
        if (!hit_test_passed.value())
            return false;
    }
    //если hit-test пройден или block_hit_test = false, проверяем hit-test детей.
    hit_list.push_back(Widget::HitListNode{ this, content_transform });
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if (it->get()->hit_test(hit_list, mouse_pos))
            return true;
    }
    if (!hit_test_passed) //нет значения => вычислим
        hit_test_passed = layout.contains(parent_transform, mouse_pos.x, mouse_pos.y);
    if (hit_test_passed.value())
        return true;
    hit_list.pop_back();
    return false;
}

Widget* Widget::add_widget(std::unique_ptr<Widget>&& child) {
    assert((!GUI::Instance().is_event_processing() || get_root() != GUI::Instance().get_root()) && "Cannot change widget hierarchy on event processing");
    child->m_parent = this;
    m_children.push_back(std::move(child));
    return m_children.back().get();
}

void Widget::delete_widget(Widget* widget, RemovePolicy policy) {
    assert((!GUI::Instance().is_event_processing() || get_root() != GUI::Instance().get_root()) && "Cannot change widget hierarchy on event processing");
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == widget) {
            (*it)->remove(policy);
            m_children.erase(it);
            return;
        }
    }
}

void Widget::delete_all_widgets(RemovePolicy policy) {
    for (auto it = m_children.begin(); it != m_children.end();) {
        it->get()->remove(policy);
        it = m_children.erase(it);
    }
}

void Widget::add_widget_deffered(std::unique_ptr<Widget>&& child) {
    Widget* child_ptr = child.release();
    GUI::Instance().add_deffered_command([this, child = child_ptr]() {
        child->m_parent = this;
        m_children.push_back(std::unique_ptr<Widget>(child));
    });
}

void Widget::delete_widget_deffered(Widget* widget, RemovePolicy policy) {
    GUI::Instance().add_deffered_command([this, widget, policy]() {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if (it->get() == widget) {
                (*it)->remove(policy);
                m_children.erase(it);
                return;
            }
        }
    });
}

void Widget::delete_dependent_rules(Property::Type props, bool hard) {
    //удаляем правила других виджетов, зависимые от нас.
    for (auto& [prop, member] : Layout::s_property_map) { //идем по всем свойствам
        if (!(prop & props))
            continue;
        for (auto& dependency : (layout.*member).dependents) { //каждое свойство знает, кто от него зависит
            for (auto rule = dependency.widget->m_rules.begin(); rule != dependency.widget->m_rules.end();) { //идем по правилам зависимого
                auto this_it = std::find_if(rule->dependencies.begin(), rule->dependencies.end(), [this](const Dependency& dependency) {
                    return dependency.widget == this;
                }); //ищем правила, зависимые от this.
                if (this_it == rule->dependencies.end()) {
                    ++rule;
                    continue;
                }
                if (!hard && rule->dependencies.size() == 1) //в случае не жесткого удаления, не можем удалить правила, зависящие от нас частично
                    throw "Cannot delete partial rule";
                dependency.widget->invalidate(rule->output); //инвализация
                dependency.widget->delete_dependent_rules(rule->output, hard); //удаляем правила, зависящие от output правила, которое мы хотим удалить
                rule = dependency.widget->m_rules.erase(rule); //удаляем
            }
        }
        (layout.*member).dependents.clear(); //больше никто от нас не зависит.
    }
}

void Widget::remove(RemovePolicy policy) {
    if (static_cast<int>(policy) & static_cast<int>(Widget::RemovePolicy::Min)) {
        invalidate(Property::LAYOUT); //заинвалидируем layout у зависимых от нас
        clear_rules(Property::LAYOUT); //удалим все правила вычисления. Это известит зависимых от нас, что мы от них больше не зависим
    }
    if (policy != Widget::RemovePolicy::Min)
        delete_dependent_rules(Property::LAYOUT, policy == Widget::RemovePolicy::DeleteDepententRulesHard);
    for (auto& child : m_children)
        child->remove(policy);
}


Widget* Widget::get_root() {
    return m_parent == nullptr ? this : m_parent->get_root();
}


Panel::Panel(sf::Color background_color, sf::Color border_color, float border) {
    m_rect.setFillColor(background_color);
    m_rect.setOutlineColor(border_color);
    m_rect.setOutlineThickness(-border);
    layout.padding = { border, border, border, border };
}

std::unique_ptr<Panel> Panel::create(sf::Color background_color, sf::Color border_color, float border) {
    return std::make_unique<Panel>(background_color, border_color, border);
}

void Panel::set_border(float border) {
    layout.padding = { border, border, border, border };
    m_rect.setOutlineThickness(-border);
}

void Panel::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    auto layout_rect = layout.get_layout_rect();
    m_rect.setSize({ layout_rect.width, layout_rect.height });
    m_rect.setPosition({ position_transform.x + layout_rect.x, position_transform.y + layout_rect.y });
    window.draw(m_rect);
}


Query Hoverable::hover_event(Widget::EventContext event_context) {
    if (event_context.event_type == Event::MOUSE_MOVED) {
        if (!event_context.from_subscribe) {
            GUI::Instance().subscribe_deffered(m_widget, Event::MOUSE_MOVED);
            return on_hovered ? on_hovered(event_context) : Query{ Query::PROCESSED }; 
        }
        else {
            if (!event_context.hit_list.empty() && event_context.hit_list.back().widget == m_widget)
                return on_mouse_moved ? on_mouse_moved(event_context) : Query{ Query::PASS };
            else {
                GUI::Instance().unsubscribe_deffered(m_widget, Event::MOUSE_MOVED);
                if (on_unhovered) {
                    size_t query = on_unhovered(event_context);
                    return Query{ Query::REPEAT, query | Query::PERFORM_DEFFERED };
                }
                else
                    return Query{ Query::REPEAT, Query::PERFORM_DEFFERED };
            }
        }
    }
    return Query{Query::PASS};
}


Query Clickable::click_event(Widget::EventContext event_context) {
    if (event_context.event_type == Event::BUTTON_PRESSED && GUI::Instance().mouse_button == m_button) {
        GUI::Instance().subscribe_deffered(m_widget, Event::BUTTON_RELEASED);
        m_clicked = true;
        return on_pressed ? on_pressed(event_context) : Query{ Query::PROCESSED };
    }
    else if (event_context.event_type == Event::BUTTON_RELEASED && GUI::Instance().mouse_button == m_button) {
        if (event_context.from_subscribe)
            GUI::Instance().unsubscribe_deffered(m_widget, Event::BUTTON_RELEASED);
        m_clicked = false;
        return on_released ? on_released(event_context) : Query{ Query::PROCESSED };
    }
    return Query{ Query::PASS };
}


