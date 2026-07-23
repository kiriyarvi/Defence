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
    m_event_processing = true;
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
    m_event_processing = false; //можно выполнять команды по изменению иерархии не отложено.
    perform_deffered(); 
    return processed;
}

bool GUI::event_impl() {
    while (true) { //REPEAT-while
        //1. hit-test
        std::list<Widget::HitListNode> hit_test;
        m_root->hit_test(hit_test, mouse_pos, event_type);
#ifdef GUI_DEBUG_ENABLED
       /* for (auto& w : hit_test) {
            std::cout << w.widget->debug_name << " -> ";
        }
        if (!hit_test.empty())
            std::cout << std::endl;*/
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
                m_event_processing = false; //можно выполнять команды по изменению иерархии не отложено.
                perform_deffered();
                m_event_processing = true; //далее продолжаем event_precessing
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
        unsubscribe(widget, type);
    });
}

void GUI::unsubscribe(Widget* widget, Event::Type type) {
    auto sub = std::find_if(m_subscribers.begin(), m_subscribers.end(), [widget](Subscriber& subscriber) {
        return subscriber.subscriber == widget;
    });
    if (sub == m_subscribers.end())
        return;
    sub->event_type &= ~type;
    if (sub->event_type == 0)
        m_subscribers.erase(sub);
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
size_t Anchor::X_CENTER = 0b010000;
size_t Anchor::Y_CENTER = 0b100000;
size_t Anchor::CENTER = 0b110000;

std::string Widget::Layout::to_string(Property::Type type) {
    static std::unordered_map<Property::Type, std::string> m{
        {Property::X, "X"},
        {Property::Y, "Y"},
        {Property::WIDTH, "WIDTH"},
        {Property::HEIGHT, "HEIGHT"}
    };
    std::vector<std::string> v;
    for (auto [prop, _] : Widget::Layout::s_property_map) {
        if (type & prop)
            v.push_back(m[prop]);
    }
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) {
        out += v[i];
        if (i != v.size() - 1)
            out += " | ";
    }
    return out;
}

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

float Margin::get_margin(float cell_size) const {
    switch (margin_function) {
    case Margin::Source::ABSOLUTE:
        return margin_source;
    case Margin::Source::FRACTION_OF_CELL:
        return margin_source * cell_size;
    case Margin::Source::FRACTION_OF_REFERENCE_WIDTH:
        return margin_source * reference->layout.width;
    case Margin::Source::FRACTION_OF_REFERENCE_HEIGHT:
        return margin_source * reference->layout.height;
    }
}

void VHBoxOptions::add_item(Widget* widget, Anchor::Type alignment) {
    items.push_back({ widget, alignment });
}

void GridOptions::resize(size_t rows, size_t cols) {
    items.resize(rows);
    for (auto& row : items)
        row.resize(cols);
}


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

/// Возвращает координаты указанного якоря прямоугольника контента относительно центра прямоугольника контента
glm::vec2 Widget::Layout::get_content_anchor_relative_to_center(Anchor::Type anchor) const {
    glm::vec2 r = { (width - padding.left - padding.right) / 2.f , (height - padding.top - padding.bottom) / 2.f };
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

/**
 * @brief Удаляем правила, вычисляющие свойства properties
 * @param properties 
 * @param notify_dependencies Оповестить виджеты, от которых мы зависим, что нам они больше не нужны. (TODO не должны нигде использовать)
 * @param clear_partial Удалить правило, даже если оно вычисляет не только свойства из properties
 * @param recursive вызвать рекурсивно для детей?
 */
void Widget::clear_rules(Property::Type properties, bool notify_dependencies, bool clear_partial, bool recursive) {
    Property::Type invalidated_props = 0;
    for (auto rule = m_rules.begin(); rule != m_rules.end();) {
        bool criteria = false;
        if (clear_partial)
            criteria = rule->output & properties; //rule->output и properties имеют хотя бы один общий флаг
        else
            criteria = (rule->output & properties) == rule->output; //rule->properties всключает все флаги что и properties
        if (criteria) {
            if (notify_dependencies) {
                for (auto& dependency : rule->dependencies) {
                    //должны сказать зависимостям, что мы больше вычисление свойств rule->properties данного виджета не зависит от них.
                    for (auto& [prop, member] : Layout::s_property_map) {
                        if (dependency.source & prop)
                            (dependency.widget->layout.*member).clear_dependent({ this, rule->output });
                    }
                }
            }
#ifdef GUI_DEBUG_ENABLED
            std::cout << "ERASE " << debug_name << " RULE for " + Layout::to_string(rule->output) << std::endl;
#endif
            invalidated_props |= rule->output;
            rule = m_rules.erase(rule);
            continue;
        }
        assert((rule->output & properties) == 0 && "Invalid operation"); //правило должно быть перпендикулярно properties.
        //если правило вычисляет X,Y, то мы не можем попросить удалить только X.
        ++rule;
    }
    invalidate(invalidated_props);
    if (recursive)
        for (auto& child : m_children)
            child->clear_rules(properties, clear_partial, recursive);
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
    m_rules.emplace_back(Rule{ output, calc, dependencies });
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
   /* std::cout << "calc " << current_frame << " [" <<
        ((property & Property::X) ? "X," : "") <<
        ((property & Property::Y) ? "Y," : "") <<
        ((property & Property::WIDTH) ? "WIDTH," : "") <<
        ((property & Property::HEIGHT) ? "HEIGHT," : "") << "] for " << debug_name << std::endl;*/
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

void Widget::x_centering(Widget* parent) {
    if (parent == nullptr)
        parent = m_parent;
    assert(parent != nullptr && "Invalid call");
    add_rule(Property::X, [parent](Layout& layout) {
        layout.x = (parent->layout.width - layout.width) / 2.f;
    }, { {parent, Property::WIDTH}, {this, Property::WIDTH} });
}

void Widget::y_centering(Widget* parent) {
    if (parent == nullptr)
        parent = m_parent;
    assert(parent != nullptr && "Invalid call");
    add_rule(Property::Y, [parent](Layout& layout) {
        layout.y = (parent->layout.height - layout.height) / 2.f;
    }, { {parent, Property::HEIGHT}, {this, Property::HEIGHT} });
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
    if (m_parent != to) { //виджеты на одном уровне и живут в одной системе координат родителя
        add_rule(Property::POSITION, [pivot, to, anchor](Layout& layout) {
            glm::vec2 anchor_pos = to->layout.get_anchor_relative_to_center(anchor) + to->layout.center();
            glm::vec2 pivot_anchor = layout.get_anchor_relative_to_center(pivot);
            glm::vec2 pos = anchor_pos - pivot_anchor - glm::vec2{ layout.width / 2.f, layout.height / 2.f };
            layout.x = pos.x;
            layout.y = pos.y;
        }, { {to, Property::LAYOUT}, {this, Property::SIZE} });
    }
    else { //иначе случай, когда to --- наш родитель
        add_rule(Property::POSITION, [pivot, to, anchor](Layout& layout) {
            glm::vec2 anchor_pos = to->layout.get_content_anchor_relative_to_center(anchor) + glm::vec2{ (to->layout.width - to->layout.padding.left - to->layout.padding.right) / 2.f, (to->layout.height - to->layout.padding.top - to->layout.padding.bottom) / 2.f};
            glm::vec2 pivot_anchor = layout.get_anchor_relative_to_center(pivot);
            glm::vec2 pos = anchor_pos - pivot_anchor - glm::vec2{ layout.width / 2.f, layout.height / 2.f };
            layout.x = pos.x;
            layout.y = pos.y;
        }, { {to, Property::SIZE}, {this, Property::SIZE} });
    }
}

void Widget::layout_contains(std::vector<Widget*> elements) {
    std::vector<Dependency> dependencies;
    dependencies.reserve(elements.size());
    for (auto& element : elements)
        dependencies.push_back({ element, Property::LAYOUT });
    add_rule(Property::LAYOUT, [elements](Layout& layout) {
        float min_x;
        float max_x;
        float min_y;
        float max_y;
        Rect sample_rect = elements[0]->layout.get_layout_rect();
        min_x = sample_rect.x;
        min_y = sample_rect.y;
        max_x = sample_rect.x + sample_rect.width;
        max_y = sample_rect.y + sample_rect.height;
        for (auto& elem : elements) {
            min_x = std::min<float>(min_x, elem->layout.x);
            min_y = std::min<float>(min_y, elem->layout.y);
            max_x = std::max<float>(max_x, elem->layout.x + elem->layout.width);
            max_y = std::max<float>(max_y, elem->layout.y + elem->layout.height);
        }
        layout.x = min_x - layout.padding.left;
        layout.y = min_y - layout.padding.right;
        layout.width = max_x - min_x + layout.padding.left + layout.padding.right;
        layout.height = max_y - min_y + layout.padding.top + layout.padding.bottom;
    }, dependencies);
}


void Widget::vhbox(const VHBoxOptions& options, bool vertical) {
    clear_rules(Property::SIZE);
    Property::Type height_prop = vertical ? Property::HEIGHT : Property::WIDTH;
    Property::Type width_prop = vertical ? Property::WIDTH : Property::HEIGHT;
    Property::Type x_prop = vertical ? Property::X : Property::Y;
    Property::Type y_prop = vertical ? Property::Y : Property::X;
    Property Layout::* height_member = vertical ? &Layout::height : &Layout::width;
    Property Layout::* width_member = vertical ? &Layout::width : &Layout::height;
    Property Layout::* x_member = vertical ? &Layout::x : &Layout::y;
    Property Layout::* y_member = vertical ? &Layout::y : &Layout::x;

    //Код пишем как для vbox, используя указатели на мемберы, подменяя их при vertical = false
    //1. вычисляем ширину и высоту контейнера
    std::vector<Dependency> height_dependencies;
    std::transform(options.items.begin(), options.items.end(), std::back_inserter(height_dependencies), [height_prop](const VHBoxOptions::Item& item) {return Dependency{ item.widget, height_prop }; });
    std::vector<Dependency> width_dependencies;
    std::transform(options.items.begin(), options.items.end(), std::back_inserter(width_dependencies), [width_prop](const VHBoxOptions::Item& item) {return Dependency{ item.widget, width_prop }; });

    height_dependencies.push_back({ this, width_prop });
    if (options.margin_function == Margin::Source::FRACTION_OF_REFERENCE_HEIGHT)
        height_dependencies.push_back({ options.reference, Property::HEIGHT });
    else if (options.margin_function == Margin::Source::FRACTION_OF_REFERENCE_WIDTH)
        height_dependencies.push_back({ options.reference, Property::WIDTH });
    add_rule(height_prop, [=](Layout& layout) {
        float height = 0.0;
        for (auto& item : options.items)
            height += item.widget->layout.*height_member;
        float width_padding = vertical ? (layout.padding.left + layout.padding.right) : (layout.padding.top + layout.padding.bottom);
        float height_padding = vertical ? (layout.padding.top + layout.padding.bottom) : (layout.padding.left + layout.padding.right);
        height += (options.items.size() - 1) * options.get_margin(layout.*width_member - width_padding);
        layout.*height_member = height + height_padding;
    }, height_dependencies);

    add_rule(width_prop, [=](Layout& layout) {
        float width = 0.0;
        for (auto& item : options.items)
            width = std::max<float>(width, item.widget->layout.*width_member);
        float width_padding = vertical ? (layout.padding.left + layout.padding.right) : (layout.padding.top + layout.padding.bottom);
        layout.*width_member = width + width_padding;
    }, width_dependencies);

    //2. Выравнивание детей.
    Widget* prev = nullptr;
    for (auto& item : options.items) {
        std::vector<Dependency> dependencies;
        if (prev)
            dependencies.push_back({ prev, y_prop | height_prop });
        dependencies.push_back({ this, width_prop });
        dependencies.push_back({ item.widget, Property::SIZE });
        item.widget->add_rule(Property::POSITION, [=, alignment = item.alignment, container = this](Layout& layout) {
            float width_padding = vertical ? (layout.padding.left + layout.padding.right) : (layout.padding.top + layout.padding.bottom);
            float cell_width = container->layout.*width_member - width_padding;
            float margin = options.get_margin(cell_width);
            layout.*y_member = prev ? (prev->layout.*y_member + prev->layout.*height_member + margin) : 0;
            if (alignment == Anchor::LEFT || alignment == Anchor::TOP)
                layout.*x_member = 0;
            else if (alignment == Anchor::RIGHT || alignment == Anchor::BOTTOM)
                layout.*x_member = cell_width - layout.*width_member;
            else //centering by default
                layout.*x_member = (cell_width - layout.*width_member) * 0.5f;
        }, dependencies);
        prev = item.widget;
    }
}

void Widget::hbox(const std::vector<Widget*>& elements, VHBoxOptions options) {
    for (auto& elem : elements)
        options.add_item(elem, options.alignment);
    hbox(options);
}

void Widget::vbox(const std::vector<Widget*>& elements, VHBoxOptions options) {
    for (auto& elem : elements)
        options.add_item(elem, options.alignment);
    vbox(options);
}

void Widget::vbox(const VHBoxOptions& options) {
    vhbox(options, true);
}

void Widget::hbox(const VHBoxOptions& options) {
    vhbox(options, false);
}

struct GridLayoutAlgorithm {
    struct CellInfo {
        Rect cell_rect;
        glm::vec2 item_pos;
    };
    GridOptions options;
    std::vector<std::vector<CellInfo>> info;

    void calc() {
        //1. Вычислим высоты ячеек
        for (size_t y = 0; y < options.items.size(); ++y) {
            auto& row = options.items[y];
            float height = 0.0;
            for (auto& item : row)
                height = std::max<float>(height, item.widget ? item.widget->layout.height : 0.0);
            for (auto& item_info : info[y])
                item_info.cell_rect.height = height;
        }
        //2. Вычислим ширины ячеек.
        for (size_t x = 0; x < options.items[0].size(); ++x) {
            float width = 0.0;
            for (size_t y = 0; y < options.items.size(); ++y)
                width = std::max<float>(width, options.items[y][x].widget ? options.items[y][x].widget->layout.width : 0.0);
            for (size_t y = 0; y < options.items.size(); ++y)
                info[y][x].cell_rect.width = width;
        }
        //вычисляем позиции ячеек
        float margin = options.get_margin(0.0);
        for (size_t y = 0; y < options.items.size(); ++y) {
            auto& row = options.items[y];
            for (size_t x = 0; x < row.size(); ++x) {
                info[y][x].cell_rect.x = (x == 0 ? 0 : (info[y][x - 1].cell_rect.x + info[y][x - 1].cell_rect.width + margin));
                info[y][x].cell_rect.y = (y == 0 ? 0 : (info[y - 1][x].cell_rect.y + info[y - 1][x].cell_rect.height + margin));
                auto& item = options.items[y][x];
                if (!item.widget)
                    continue;
                auto& item_info = info[y][x];
                Anchor::Type alignment = item.alignment;
                if (alignment & Anchor::LEFT)
                    item_info.item_pos.x = 0;
                else if (alignment & Anchor::RIGHT)
                    item_info.item_pos.x = item_info.cell_rect.width - item.widget->layout.width;
                else if (alignment & Anchor::X_CENTER)
                    item_info.item_pos.x = (item_info.cell_rect.width - item.widget->layout.width) / 2.f;

                if (alignment & Anchor::TOP)
                    item_info.item_pos.y = 0;
                else if (alignment & Anchor::BOTTOM)
                    item_info.item_pos.y = item_info.cell_rect.height - item.widget->layout.height;
                else if (alignment & Anchor::Y_CENTER)
                    item_info.item_pos.y = (item_info.cell_rect.height - item.widget->layout.height) / 2.f;
            }
        }
    }

    void operator()(Widget::Layout& layout){
        calc();
        auto& last_item = info.back().back();
        layout.width = last_item.cell_rect.x + last_item.cell_rect.width + layout.padding.left + layout.padding.right;
        layout.height = last_item.cell_rect.y + last_item.cell_rect.height + layout.padding.top + layout.padding.bottom;
    }
};



void Widget::grid(const GridOptions& options) {
    auto algorithm = std::make_shared<GridLayoutAlgorithm>();
    algorithm->info.resize(options.items.size());
    for (size_t y = 0; y < algorithm->info.size(); ++y)
        algorithm->info[y].resize(options.items[y].size());
    algorithm->options = options;

    std::vector<Dependency> dependencies;
    for (auto& row : options.items)
        for (auto& item : row)
            if (item.widget)
                dependencies.push_back({ item.widget, Property::SIZE });
    if (options.margin_function == Margin::Source::FRACTION_OF_REFERENCE_HEIGHT)
        dependencies.push_back({ options.reference, Property::HEIGHT });
    else if (options.margin_function == Margin::Source::FRACTION_OF_REFERENCE_WIDTH)
        dependencies.push_back({ options.reference, Property::WIDTH });
    add_rule(Property::SIZE, [algorithm](Layout& layout) {
        (*algorithm)(layout);
    }, dependencies);

    for (size_t y = 0; y < options.items.size(); ++y)
        for (size_t x = 0; x < options.items[y].size(); ++x)
            if (options.items[y][x].widget)
                options.items[y][x].widget->add_rule(Property::POSITION, [y, x, algorithm](Layout& layout) {
                    auto& cached_info = algorithm->info[y][x];
                    layout.x = cached_info.item_pos.x + cached_info.cell_rect.x;
                    layout.y = cached_info.item_pos.y + cached_info.cell_rect.y;
                }, { {this, Property::SIZE} });
}

void Widget::grid(const std::vector<std::vector<Widget*>> elements, GridOptions options) {
    int rows = elements.size();
    int cols = elements[0].size();
    options.resize(rows, cols);
    for (size_t y = 0; y < rows; ++y) for (size_t x = 0; x < cols; ++x) {
        options.items[y][x].widget = elements[y][x];
        options.items[y][x].alignment = options.alignment;
    }
    grid(options);
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

bool Widget::hit_test(std::list<Widget::HitListNode>& hit_list, glm::uvec2 mouse_pos, Event::Type event_type) {
    if (hit_test_policy == HitTestPolicy::Terminate || (transparent_for & event_type))
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
        if (it->get()->hit_test(hit_list, mouse_pos, event_type))
            return true;
    }
    if (!hit_test_passed) //нет значения => вычислим
        hit_test_passed = layout.contains(parent_transform, mouse_pos.x, mouse_pos.y);
    if (hit_test_passed.value())
        return true;
    hit_list.pop_back();
    return false;
}

//Проверяет что среди завивимостей этой иерархии есть данный виджет
bool Widget::contain_dependency(Widget* widget) {
    if (this == widget)
        return false;
    for (auto& rule : m_rules) {
        for (auto& dependency : rule.dependencies)
            if (dependency.widget == widget)
                return true;
    }
    for (auto& child : m_children)
        if (child->contain_dependency(widget))
            return true;
    return false;
}


/**
 * @brief Удаляет виджет вместе с его потомками.
 * У виджета и его потомков перед удалением удаляются все правила.
 * Также удаляются подписки виджета и его потомков.
 */
void Widget::delete_widget(Widget* widget) {
    if (widget == nullptr)
        return;
    assert((!GUI::Instance().is_event_processing() || get_root() != GUI::Instance().get_root()) && "Cannot change widget hierarchy on event processing");
    auto it = std::find_if(m_children.begin(), m_children.end(), [widget](std::unique_ptr<Widget>& child) {return child.get() == widget; });
    assert(it != m_children.end() && "Widget is no a child of this widget");
    widget->clear_rules(Property::LAYOUT, true, false, true); //Удалим правила из всей иерархии
    assert(!GUI::Instance().get_root()->contain_dependency(widget) && "Hanging rule");
    widget->delete_all_widgets_impl(); //удаляем детей
    GUI::Instance().unsubscribe(widget, Event::ANY); //отписываемся
    auto w = std::move(*it);
    m_children.erase(it); //достали виджет из иерархии, теперь он полностью от нее независим.
    w.reset();//можем удалить, во время удаления виджет может пытаться менять дерево виджетов
    //поскольку оно на текущий момент корректно.
}

void Widget::delete_all_widgets() {
    assert((!GUI::Instance().is_event_processing() || get_root() != GUI::Instance().get_root()) && "Cannot change widget hierarchy on event processing");
    for (auto& child : m_children)
        child->clear_rules(Property::LAYOUT, true, false, true); //Удалим правила из всей иерархии
    delete_all_widgets_impl();
}

void Widget::delete_all_widgets_impl() {
#ifdef GUI_DEBUG_ENABLED
    //при вызове этой функции не должно быть никаких "высячих" правил
    for (auto& child : m_children)
        assert(!GUI::Instance().get_root()->contain_dependency(child.get()) && "Hanging rule");
#endif
    for (auto& child : m_children)
        child->delete_all_widgets();
    for (auto& child : m_children)
        GUI::Instance().unsubscribe(child.get(), Event::ANY); //отписать всех  
    std::list<std::unique_ptr<Widget>> children = std::move(m_children); //вынимаем детей из иерархии
    children.clear(); //удаляем их.
}

void Widget::add_children(std::list<Widget*>& list) {
    for (auto& child : m_children)
        list.push_back(child.get());
    for (auto& child : m_children)
        child->add_children(list);
}

bool Widget::check_for_external_referencies(std::list<Widget*>& hierarchy) {
    for (auto& rule : m_rules)
        for (auto dependency : rule.dependencies) {
            if (std::find(hierarchy.begin(), hierarchy.end(), dependency.widget) == hierarchy.end())
                return true;
        }
    for (auto& child : m_children)
        if (child->check_for_external_referencies(hierarchy))
            return true;
    return false;
}

//проверяет есть ли в иерархии виджетов, начинающейся с данного виджета, правила вычисления по виджетам не из данной иерархии
bool Widget::check_for_external_referencies() {
    std::list<Widget*> hierarchy;
    add_children(hierarchy);
    return check_for_external_referencies(hierarchy);
}

std::unique_ptr<Widget> Widget::release(Widget* widget) {
    assert(!widget->check_for_external_referencies() && "There should be no external referencies. You need to delete them by clear_rule call.");
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == widget) {
            std::unique_ptr<Widget> out = std::move(*it);
            it = m_children.erase(it);
            return out;
        }
    }
    return nullptr;
}



void Widget::delete_widget_deffered(Widget* widget) {
    GUI::Instance().add_deffered_command([this, widget]() {
        delete_widget(widget);
    });
}

/// В зависимости от того, происходит ли сейчас обработка событий или нет
/// вызывает либо delete_widget либо delete_widget_deffered
void Widget::delete_widget_smart(Widget* widget) {
    if (GUI::Instance().is_event_processing())
        delete_widget_deffered(widget);
    else
        delete_widget(widget);
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


Query HoverableWidget::on_event(EventContext event_context) {
    on_hovered_return = Query{ Query::PROCESSED };
    on_unhovered_return = Query{ Query::REPEAT, Query::PERFORM_DEFFERED };
    on_mouse_moved_return = Query{ Query::PROCESSED };
    if (event_context.event_type == Event::MOUSE_MOVED) {
        if (!m_hovered) {
            if (capture_mouse_move_only_if_no_obstacles && event_context.hit_list.back().widget != this)
                return Query{ Query::PASS };
            GUI::Instance().subscribe_deffered(this, Event::MOUSE_MOVED);
            m_hovered = true;
            if (m_on_hovered)
                m_on_hovered();
            return on_hovered_return;
        }
        else {
            bool hit_test;
            if (!capture_mouse_move_only_if_no_obstacles)
                hit_test = std::find_if(event_context.hit_list.begin(), event_context.hit_list.end(), [this](const Widget::HitListNode& node) { return node.widget == this; }) != event_context.hit_list.end();
            else
                hit_test = !event_context.hit_list.empty() && event_context.hit_list.back().widget == this;
            if (hit_test) {
                if (m_on_mouse_moved)
                    m_on_mouse_moved();
                return on_mouse_moved_return;
            }
            else {
                GUI::Instance().unsubscribe_deffered(this, Event::MOUSE_MOVED);
                m_hovered = false;
                if (m_on_unhovered) {
                    m_on_unhovered();
                }
                return on_unhovered_return;
            }
        }
    }
    return Query{ Query::PASS };
}


void ClickableWidget::enabled(Button::Type enabled) {
    if (enabled == m_enabled)
        return;
    if (((~enabled & m_clicked) == m_clicked && m_clicked) ) { //то есть отключаем всё, на что ждем release
        GUI::Instance().unsubscribe_deffered(this, Event::BUTTON_RELEASED);
        m_clicked = 0;
    }
    m_enabled = enabled;
}

ClickableWidget::Button::Type ClickableWidget::button_type_from(sf::Mouse::Button button) {
    if (button == sf::Mouse::Left)
        return Button::LEFT;
    else if (button == sf::Mouse::Right)
        return Button::RIGHT;
    else
        return 0;
}

Query ClickableWidget::on_event(EventContext event_context) {
    on_pressed_return = Query{ Query::PROCESSED };
    on_released_return = Query{ Query::PROCESSED };
    if (!m_enabled) // если не активны, не взаимодействуем.
        return Query{ Query::PASS };
    Button::Type mouse_button = button_type_from(GUI::Instance().mouse_button);
    if (event_context.event_type == Event::BUTTON_PRESSED && (mouse_button & m_button)) { //если нажали кнопку
        if (capture_mode)
            GUI::Instance().subscribe_deffered(this, Event::BUTTON_RELEASED | Event::MOUSE_MOVED); //подпись
        m_clicked |= mouse_button; //указываем, что нажали
        if (m_on_pressed)
            m_on_pressed(mouse_button); //отправляем только то, что нажали, а не накопленное
        return on_pressed_return;
    }
    else if (event_context.event_type == Event::BUTTON_RELEASED && (mouse_button & m_button)) {
        if (m_clicked) {
            if (capture_mode)
                GUI::Instance().unsubscribe_deffered(this, Event::BUTTON_RELEASED);
            if (m_on_released)
                m_on_released(mouse_button);
            m_clicked &= ~mouse_button;
            return on_released_return;
        } //else - skip - кнопку зажали, навели на нас и отпустили или по какой-то причине мы не словили BUTTON_PRESSED (например виджет перед нами его перехватил) => игнорируем
    }
    return Query::skip(event_context.from_subscribe);
}


Query HoverableClickableWidget::on_event(Widget::EventContext event_context) {
    on_hovered_return = Query{ Query::PROCESSED };
    on_unhovered_return = Query{ Query::REPEAT, Query::PERFORM_DEFFERED };
    on_mouse_moved_return = Query{ Query::PROCESSED };
    on_pressed_return = Query{ Query::PROCESSED };
    on_released_return = Query{ Query::PROCESSED };
    Button::Type mouse_button = button_type_from(GUI::Instance().mouse_button);
    if (event_context.event_type == Event::BUTTON_PRESSED && (mouse_button & m_button)) { //если нажали кнопку
        if (!m_enabled) // если не активны, не взаимодействуем.
            return Query{ Query::PASS };
        if (capture_mode)
            GUI::Instance().subscribe_deffered(this, Event::BUTTON_RELEASED | Event::MOUSE_MOVED); //подпись
        m_clicked |= mouse_button;
        if (unhover_on_pressed && m_hovered) {
            m_hovered = false;
            if (m_on_unhovered)
                m_on_unhovered();
        }
        if (m_on_pressed)
            m_on_pressed(mouse_button); //отправляем только то, что нажали, а не накопленное
        return on_pressed_return;
    }
    else if (event_context.event_type == Event::BUTTON_RELEASED && (mouse_button & m_button)) {
        if (!m_enabled) // если не активны, не взаимодействуем.
            return Query{ Query::PASS };
        if (m_clicked) {
            if (capture_mode)
                GUI::Instance().unsubscribe_deffered(this, Event::BUTTON_RELEASED);
            if (m_on_released)
                m_on_released(mouse_button);
            m_clicked &= ~mouse_button;
            return on_released_return;
        } //else - skip - кнопку зажали, навели на нас и отпустили или по какой-то причине мы не словили BUTTON_PRESSED (например виджет перед нами его перехватил) => игнорируем
    } else if (event_context.event_type == Event::MOUSE_MOVED) {
        if (m_clicked && unhover_on_pressed)
            return Query::skip(event_context.from_subscribe);
        if (m_hovered) {
            bool hit_test;
            if (!capture_mouse_move_only_if_no_obstacles)
                hit_test = std::find_if(event_context.hit_list.begin(), event_context.hit_list.end(), [this](const Widget::HitListNode& node) { return node.widget == this; }) != event_context.hit_list.end();
            else
                hit_test = !event_context.hit_list.empty() && event_context.hit_list.back().widget == this;
            if (hit_test) {
                if (m_on_mouse_moved)
                    m_on_mouse_moved();
                return on_mouse_moved_return;
            }
            else {
                GUI::Instance().unsubscribe_deffered(this, Event::MOUSE_MOVED);
                m_hovered = false;
                if (m_on_unhovered) {
                    m_on_unhovered();
                }
                return on_unhovered_return;
            }
        }
        else {
            if (capture_mouse_move_only_if_no_obstacles && event_context.hit_list.back().widget != this)
                return Query{ Query::PASS };
            GUI::Instance().subscribe_deffered(this, Event::MOUSE_MOVED);
            m_hovered = true;
            if (m_on_hovered)
                m_on_hovered();
            return on_hovered_return;
        }
    }
    return Query::skip(event_context.from_subscribe);
}
