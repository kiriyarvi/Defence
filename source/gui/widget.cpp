#include "gui/widget.h"
#include <numeric>
#include <iostream>

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

void GUI::event(const sf::Event& event) {
    if (!m_root)
        return;
    if (event.type == sf::Event::Resized) {
        window_size = { event.size.width , event.size.height };
        m_root->invalidate(Property::SIZE);
        return;
    } else if (event.type == sf::Event::MouseMoved) {
        mouse_pos = { event.mouseMove.x, event.mouseMove.y };
        //функции on_unhovered/on_hovered могут менять дерево виджетов, поэтому после выдова таких функций
        //данные полученные из get_widget_under_cursor становятся неактуальными.
        //последствия этого мы обработаем при следующем событии.
        //то, что было совершено удаление, можно узнать по флагу m_invalidated.
        do {
            auto [widget_under_cursor, transform] = m_root->get_widget_under_cursor({ 0,0 }, mouse_pos);
            auto [old_hovered, old_hovered_transform] = m_hovered;
            if (widget_under_cursor != old_hovered) {
                if (old_hovered) {
                    m_invalidated = false;
                    m_hovered = { nullptr, glm::vec2{} };
                    if (old_hovered->on_unhovered) {
                        old_hovered->on_unhovered(old_hovered_transform, mouse_pos);
                        if (m_invalidated)
                            continue; //layout изменился, придется сделать новый запрос.
                    }
                    if (widget_under_cursor) { //layout не менялся, можем привязывать новый виджет, если он есть
                        m_hovered = { widget_under_cursor, transform };
                        if (widget_under_cursor->on_hovered) {
                            widget_under_cursor->on_hovered(transform, mouse_pos);
                            if (m_invalidated)
                                continue; //делаем новый заброс get_widget_under_cursor
                        }
                    }
                } else if (widget_under_cursor) {
                    m_invalidated = false;
                    m_hovered = { widget_under_cursor, transform };
                    if (widget_under_cursor->on_hovered) {
                        widget_under_cursor->on_hovered(transform, mouse_pos);
                        if (m_invalidated)
                            continue; //делаем новый заброс get_widget_under_cursor
                    }
                }
                else {
                    m_invalidated = false;
                    m_hovered = { nullptr, glm::vec2{} };
                }
            }
            else if (widget_under_cursor && widget_under_cursor->on_mouse_moved) {
                m_invalidated = false;
                widget_under_cursor->on_mouse_moved(transform, mouse_pos);
            }
            else
                m_invalidated = false;
        } while (m_invalidated);
    }
    else if (event.type == sf::Event::MouseButtonPressed) {
        auto [widget_under_cursor, transform] = m_root->get_widget_under_cursor({ 0,0 }, mouse_pos);
        if (widget_under_cursor && widget_under_cursor->on_pressed)
            widget_under_cursor->on_pressed(transform, mouse_pos, event.mouseButton);
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        auto [widget_under_cursor, transform] = m_root->get_widget_under_cursor({ 0,0 }, mouse_pos);
        if (widget_under_cursor && widget_under_cursor->on_released)
            widget_under_cursor->on_released(transform, mouse_pos, event.mouseButton);
    }
}

void GUI::invalidate_event_states() {
    m_invalidated = true;
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

//Dependent = {виджет, какие поля виджета зависят от каких полей }
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

/// Удаляет правила вычисления свойств properties.
void Widget::clear_rules(Property::Type properties) {
    for (auto it = m_rules.begin(); it != m_rules.end();) {
        if ((it->properties & properties) == it->properties) { //it->properties всключает все флаги что и properties
            for (auto& dependency : it->dependencies) {
                //должны сказать зависимостям, что мы больше не завсим от их свойств dependency.properties.
                if (dependency.properties & Property::X) //зависили от свойства X
                    dependency.widget->layout.x.clear_dependent({ this, it->properties }); //говорим свойству X виджета dependency.widget, что мы от него больше не зависим.
                if (dependency.properties & Property::Y)
                    dependency.widget->layout.y.clear_dependent({ this, it->properties });
                if (dependency.properties & Property::WIDTH)
                    dependency.widget->layout.width.clear_dependent({ this, it->properties });
                if (dependency.properties & Property::HEIGHT)
                    dependency.widget->layout.height.clear_dependent({ this, it->properties });
            }
            it = m_rules.erase(it);            
            continue;
        }
        assert((it->properties & properties) == 0 && "Invalid operation"); //правило должно быть перпендикулярно properties.
        //если правило вычисляет X,Y, то мы не можем попросить удалить только X.
        ++it;
    }
}


/**
 * @brief Добавляет новое правило вычисления.
 * @param properties Свойства, для которых будет применяться аднное правило
 * @param calc функция, вычисляющая данные свойства
 * @param dependencies зависимости, т.е. виджеты и их свойства, которые должны быть вычислены на момент выполнения calc.
 * @details удаляет правила, которые ранее вычисляли свойства properties.
 * @note поскольку эта функция используется в основном при инициализации,
 * соотвествующие свойства не инвализируются. Это сделано для избежания множества
 * холостых вызовов invalidate.
 */
void Widget::add_rule(Property::Type properties, const std::function<void(Layout&)>& calc, const std::vector<Dependency>& dependencies) {
    clear_rules(properties);
    m_rules.push_back({ properties, calc, dependencies });
    for (auto& dependency : dependencies) {
        if (dependency.properties & Property::X) //зависили от свойства X
            dependency.widget->layout.x.add_depentent({ this, properties }); //говорим свойству X виджета dependency.widget, что мы от него больше не зависим.
        if (dependency.properties & Property::Y)
            dependency.widget->layout.y.add_depentent({ this, properties });
        if (dependency.properties & Property::WIDTH)
            dependency.widget->layout.width.add_depentent({ this, properties });
        if (dependency.properties & Property::HEIGHT)
            dependency.widget->layout.height.add_depentent({ this, properties });
    }
}

/// Вычисляет свойства виджета (свойства - это x,y,width, height), указанные в битовой маске property.
void Widget::calc_properties(Property::Type property) {
#ifdef GUI_DEBUG_ENABLED
    auto& stack = GUI::Instance().layout_stack;
    assert(std::find(stack.begin(), stack.end(), GUI::StackElement{ this, property }) == stack.end());
    stack.push_back(GUI::StackElement{ this, property });
#endif

    auto current_frame = GUI::Instance().get_current_frame_id();
    //не будем вычислять то, что не было инвалидировано и что уже было вычислено на этом этапе.
    Property::Type invalidated_props = layout.invalidated_props();
    
    if (!(invalidated_props & Property::X) || layout.x.last_calculation_frame_id == current_frame)
        property &= ~Property::X;
    if (!(invalidated_props & Property::Y) || layout.y.last_calculation_frame_id == current_frame)
        property &= ~Property::Y;
    if (!(invalidated_props & Property::WIDTH) || layout.width.last_calculation_frame_id == current_frame)
        property &= ~Property::WIDTH;
    if (!(invalidated_props & Property::HEIGHT) || layout.height.last_calculation_frame_id == current_frame)
        property &= ~Property::HEIGHT;
    if (property == 0) {
#ifdef GUI_DEBUG_ENABLED
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
    for (auto& r : m_rules) {
        if ((r.properties & property) != 0) {
            for (auto& d : r.dependencies) {
                d.widget->calc_properties(d.properties);
            }
            //не дадим calc_function менять то, на что она не претендует.
            layout.x.locked = !(r.properties & Property::X);
            layout.y.locked = !(r.properties & Property::Y);
            layout.width.locked = !(r.properties & Property::WIDTH);
            layout.height.locked = !(r.properties & Property::HEIGHT);

            r.calc_function(layout);
            layout.m_invalidated_props &= ~(r.properties & property); //validation.

            //обновляем информацию и frame для вычисленных полей
            if (!layout.x.locked) layout.x.last_calculation_frame_id = current_frame;
            if (!layout.y.locked) layout.y.last_calculation_frame_id = current_frame;
            if (!layout.width.locked) layout.width.last_calculation_frame_id = current_frame;
            if (!layout.height.locked) layout.height.last_calculation_frame_id = current_frame;

        }
    }
    //для некоторых свойств не нашлось правил для их вычисления => применяем правила по умолчанию
    Property::Type required = layout.m_invalidated_props & property;
    if (required != 0) {
        if (required & Property::X) {
            layout.x.locked = false;
            layout.x = 0;
            layout.x = current_frame;
        }
        if (required & Property::Y) {
            layout.y.locked = false;
            layout.y = 0;
            layout.y = current_frame;
        }
        assert((required & Property::WIDTH)== 0 && "no rule for WIDTH");
        assert((required & Property::HEIGHT) == 0 && "no rule for HEIGHT");
        layout.m_invalidated_props &= ~required; //validate
    }

#ifdef GUI_DEBUG_ENABLED
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
    layout.invalidate(property);
    if (property & Property::X) {
        for (auto& dependent : layout.x.dependents) {
            //у зависимого нужно инвалидировать те свойства, которые вычисляются по this и которые мы хотим инвалидировать (т.е. property). 
            dependent.widget->invalidate(dependent.output);
        }
    }
    if (property & Property::Y) {
        for (auto& dependent : layout.y.dependents) {
            dependent.widget->invalidate(dependent.output);
        }
    }
    if (property & Property::WIDTH) {
        for (auto& dependent : layout.width.dependents) {
            dependent.widget->invalidate(dependent.output);
        }
    }
    if (property & Property::HEIGHT) {
        for (auto& dependent : layout.height.dependents) {
            dependent.widget->invalidate(dependent.output);
        }
    }
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

/// Устанавливает фиксированный размер
void Widget::size_fixed(float width, float height) {
    add_rule(Property::SIZE, [width, height](Layout& properties) {
        properties.height = height;
        properties.width = width;
    }, {});
}

/// Свойства properties у данного виджета будут получаться из аналогичных свойств content_rect widget, промущенных через modifier.
void Widget::property_inherit(Widget* widget, Property::Type properties, const Modifier& modifier) {
    assert((properties & Property::POSITION) == 0 && "For size properties only");
    add_rule(properties, [widget, properties, modifier](Layout& layout) {
        Rect content = widget->layout.get_content_rect();
        if (properties & Property::WIDTH)
            layout.width = modifier ? modifier(content.width) : content.width;
        if (properties & Property::HEIGHT)
            layout.height = modifier ? modifier(content.height) : content.height;
    }, { {widget, properties} });
}

/// Данная функция применяется только к свойствам, относящимся к свойствам размера виджета.
/// У widget берутся свойства properties и пропускаются через modifier.
/// на выходе получается некоторые свойства размера S. У данного виджета устанавливаются свойства размера
/// так, чтобы аналогичные свойства контента были как S.
void Widget::property_include(Widget* widget, Property::Type properties, const Modifier& modifier) {
    assert((properties & Property::POSITION) == 0 && "For size properties only");
    add_rule(properties, [widget, properties, modifier](Layout& layout) {
        glm::vec2 size = layout.layout_size({ widget->layout.width, widget->layout.height });
        if (properties & Property::WIDTH)
            layout.width = modifier ? modifier(size.x) : size.x;
        if (properties & Property::HEIGHT)
            layout.height = modifier ? modifier(size.y) : size.y;
    }, { {widget, properties} });
}

///центрирует виджет внутри parent. parent может быть любым виджетом (по умолчанию всегда родитель), но
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

///short-cut для property_inherit(parent, Property::SIZE)
void Widget::size_inherited(Widget* widget) {
    property_inherit(widget, Property::SIZE);
}

///short-cut для property_inherit(parent, Property::SIZE)
void Widget::size_include(Widget* widget) {
    property_include(widget, Property::SIZE);
}

///width и height данного виджета составляют parent_width_fraction, parent_height_fraction от размеров widget.
void Widget::size_fraction(Widget* widget, float parent_width_fraction, float parent_height_fraction) {
    add_rule(Property::SIZE, [widget, parent_width_fraction, parent_height_fraction](Layout& properties) {
        auto content = widget->layout.get_content_rect();
        properties.width = content.width * parent_width_fraction;
        properties.height = content.height * parent_height_fraction;
    }, { {widget, Property::SIZE} });
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

std::unique_ptr<Widget> Widget::create(Widget* parent) {
    return std::make_unique<Widget>(parent);
}

void Widget::draw_hierarchy(int frame, const glm::vec2& position_transform, sf::RenderWindow& window) {
    calc_layout();
    draw(position_transform, window);
    glm::vec2 transform = position_transform + glm::vec2{layout.x + layout.padding.left, layout.y + layout.padding.top};
    for (auto& child : m_children) {
        child->draw_hierarchy(frame, child->layout.absolute ? glm::vec2(0.f, 0.f) : transform, window);
    }
}

std::pair<Widget*, glm::vec2> Widget::get_widget_under_cursor(const glm::vec2& parent_transform, glm::uvec2 mouse_pos) {
    if (!receive_mouse_events)
        return std::make_pair<Widget*, glm::vec2>(nullptr, glm::vec2{});
    glm::vec2 transform = parent_transform + glm::vec2{ layout.x + layout.padding.left, layout.y + layout.padding.top };
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        auto info = (*it)->get_widget_under_cursor(transform, mouse_pos);
        if (info.first) return info;
    }
    if (layout.contains(parent_transform, mouse_pos.x, mouse_pos.y))
        return std::make_pair<Widget*, glm::vec2>(this, glm::vec2(parent_transform));
    return std::make_pair<Widget*, glm::vec2>(nullptr, glm::vec2{});
}

Widget* Widget::add(std::unique_ptr<Widget>&& child) {
    GUI::Instance().invalidate_event_states();
    child->m_parent = this;
    m_children.push_back(std::move(child));
    return m_children.back().get();
}

void Widget::delete_widget(Widget* widget) {
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == widget) {
            it->reset();
            m_children.erase(it);
            GUI::Instance().invalidate_event_states();
            return;
        }
    }
}

Widget::~Widget() {
    invalidate(Property::LAYOUT); //заинвалидируем layout у зависимых от нас
    clear_rules(Property::LAYOUT); //удалим все правила вычисления. Это известит зависимых от нас, что мы от них больше не зависим
    m_children.clear(); //удаляем детей
    //уничтожаемся сами
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

void Panel::draw(const glm::vec2& position_transform, sf::RenderWindow& window) {
    auto layout_rect = layout.get_layout_rect();
    m_rect.setSize({ layout_rect.width, layout_rect.height });
    m_rect.setPosition({ position_transform.x + layout_rect.x, position_transform.y + layout_rect.y });
    window.draw(m_rect);
}
