#include "gui/widget.h"
#include <numeric>


void GUI::set_root(std::unique_ptr<Widget>&& root, const sf::RenderWindow& window) {
    m_root = std::move(root);
    m_root->size_fixed(window.getSize().x, window.getSize().y);
}

void GUI::draw(sf::RenderWindow& window) {
    if (frame == std::numeric_limits<size_t>::max()) {
        frame = 0;
    }
    ++frame;
    if (m_root) {
        auto view = window.getView();
        auto w_size = window.getSize();
        window.setView(sf::View(sf::FloatRect(0, 0, w_size.x, w_size.y)));
        m_root->draw_hierarchy(frame, { 0,0 }, window);
        window.setView(view);
    }
}

void GUI::event(const sf::Event& event) {
    if (!m_root)
        return;
    if (event.type == sf::Event::Resized) {
        m_root->size_fixed(event.size.width, event.size.height);
        return;
    }
    Widget* new_hovered = m_root->get_hovered({ 0,0 }, { sf::Mouse::getPosition().x, sf::Mouse::getPosition().y });
    if (new_hovered != hovered && hovered)
        hovered->on_unhovered();
    if (new_hovered)
        new_hovered->on_hovered();
    hovered = new_hovered;

    m_root->event_hierarchy({0,0}, event);
}

Property::Type Property::X =       0x0001;
Property::Type Property::Y =       0x0010;
Property::Type Property::WIDTH =   0x0100;
Property::Type Property::HEIGHT =  0x1000;
Property::Type Property::SIZE =    0x1100;
Property::Type Property::POSITION =0x0011;
Property::Type Property::LAYOUT =  0x1111;

size_t Anchor::LEFT = 0x1000;
size_t Anchor::RIGHT = 0x0100;
size_t Anchor::TOP = 0x0010;
size_t Anchor::BOTTOM = 0x0001;
size_t Anchor::CENTER = 0;


LayoutNode::Rect LayoutNode::Layout::get_content_rect() const {
    Rect r;
    r.x = x + padding.left;
    r.y = y + padding.top;
    r.height = height - padding.top - padding.bottom;
    r.width = width - padding.left - padding.right;
    return r;
}

LayoutNode::Rect LayoutNode::Layout::get_layout_rect() const {
    Rect r;
    r.x = x;
    r.y = y;
    r.height = height;
    r.width = width;
    return r;
}


glm::vec2 LayoutNode::Layout::layout_size(const glm::vec2& content_size) const {
    return {
        content_size.x + padding.left + padding.right,
        content_size.y + padding.top + padding.bottom
    };
}

//gets anchor relative to center
glm::vec2 LayoutNode::Layout::get_anchor_relative_to_center(Anchor::Type anchor) const {
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

glm::vec2 LayoutNode::Layout::center() const {
    return { x + width / 2.f, y + height / 2.f };
}

bool LayoutNode::Layout::contains(const glm::vec2& transform, float px, float py) {
    px -= transform.x;
    py -= transform.y;
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

void LayoutNode::clear_rules(Property::Type properties) {
    for (auto it = rules.begin(); it != rules.end();) {
        //нельзя удалить правило частично.
        if ((it->properties & properties) == it->properties) { //it->properties всключает все флаги что и properties
            it = rules.erase(it);
            continue;
        }   
        assert((it->properties & properties) == 0 && "Invalid operation");
        ++it;
    }
}

void LayoutNode::add_rule(Property::Type properties, const std::function<void(Layout&)>& calc, const std::vector<Rule::Dependency>& dependencies) {
    clear_rules(properties);
    rules.push_back({ properties, calc, dependencies });
}

void LayoutNode::calc_properties(Property::Type property) {
#ifdef GUI_DEBUG_ENABLED
    auto& stack = GUI::Instance().layout_stack;
    assert(std::find(stack.begin(), stack.end(), GUI::StackElement{ this, property }) == stack.end());
    stack.push_back(GUI::StackElement{ this, property });
#endif

    auto current_frame = GUI::Instance().get_current_frame_id();
    //не будем вычислять то, что уже было вычислено на этом этапе.
    if (layout.x.last_calculation_frame_id == current_frame)
        property &= ~Property::X;
    if (layout.y.last_calculation_frame_id == current_frame)
        property &= ~Property::Y;
    if (layout.width.last_calculation_frame_id == current_frame)
        property &= ~Property::WIDTH;
    if (layout.width.last_calculation_frame_id == current_frame)
        property &= ~Property::HEIGHT;
    //выполним все правила, которые вычисляют properties пересекающиеся с property.
    for (auto& r : rules) {
        if ((r.properties & property) != 0) {
            for (auto& d : r.dependencies) {
                d.layout_node->calc_properties(d.properties);
            }
            //не дадим calc_function менять то, на что она не претендует.
            layout.x.locked = !(r.properties & Property::X);
            layout.y.locked = !(r.properties & Property::Y);
            layout.width.locked = !(r.properties & Property::WIDTH);
            layout.height.locked = !(r.properties & Property::HEIGHT);

            r.calc_function(layout);
            //обновляем информацию и frame для вычисленных полей
            if (!layout.x.locked) layout.x.last_calculation_frame_id = current_frame;
            if (!layout.y.locked) layout.y.last_calculation_frame_id = current_frame;
            if (!layout.width.locked) layout.width.last_calculation_frame_id = current_frame;
            if (!layout.height.locked) layout.height.last_calculation_frame_id = current_frame;

        }
    }
    

#ifdef GUI_DEBUG_ENABLED
    stack.pop_back();
#endif
}


void LayoutNode::calc_layout() {
    calc_properties(Property::LAYOUT);
}


void LayoutNode::vbox(const std::vector<LayoutNode*>& elements) {
    for (size_t i = 1; i < elements.size(); ++i) {
        elements[i]->add_rule(Property::POSITION, [prev = elements[i - 1]](Layout& layout) {
            layout.x = prev->layout.x;
            layout.y = prev->layout.y + prev->layout.height;
        }, { { elements[i - 1], Property::POSITION | Property::HEIGHT } });   
    }

    std::vector<Rule::Dependency> height_dependencies(elements.size());
    std::vector<Rule::Dependency> width_dependencies(elements.size());
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

void LayoutNode::size_fixed(float width, float height) {
    add_rule(Property::SIZE, [width, height](Layout& properties) {
        properties.height = height;
        properties.width = width;
    }, {});
}

void LayoutNode::property_inherit(LayoutNode* parent, Property::Type properties, const Modifier& modifier) {
    assert((properties & Property::POSITION) == 0 && "For size properties only");
    add_rule(properties, [parent, properties, modifier](Layout& layout) {
        Rect content = parent->layout.get_content_rect(); 
        if (properties & Property::WIDTH)
            layout.width = modifier ? modifier(content.width) : content.width;
        if (properties & Property::HEIGHT)
            layout.height = modifier ? modifier(content.height) : content.height;
    }, { {parent, properties} });
}

void LayoutNode::property_include(LayoutNode* child, Property::Type properties, const Modifier& modifier) {
    assert((properties & Property::POSITION) == 0 && "For size properties only");
    add_rule(properties, [child, properties, modifier](Layout& layout) {
        glm::vec2 size = layout.layout_size({ child->layout.width, child->layout.height });
        if (properties & Property::WIDTH)
            layout.width = modifier ? modifier(size.x) : size.x;
        if (properties & Property::HEIGHT)
            layout.height = modifier ? modifier(size.y) : size.y;
    }, { {child, properties} });
}


void LayoutNode::position_centering(LayoutNode* parent) {
    add_rule(Property::POSITION, [parent](Layout& layout) {
        layout.x = (parent->layout.width - layout.width) / 2.f;
        layout.y = (parent->layout.height - layout.height) / 2.f;
    }, { {parent, Property::SIZE}, {this, Property::SIZE} });
}

void LayoutNode::position_tooltip(LayoutNode* parent, size_t ancher) {
    add_rule(Property::POSITION, [parent, ancher](Layout& layout) {
        auto position = glm::vec2{ sf::Mouse::getPosition().x,sf::Mouse::getPosition().y } - layout.get_anchor_relative_to_center(ancher);
        auto content = parent->layout.get_content_rect();
        if (position.x + layout.width > content.width)
            position.x = content.width - layout.width;
        if (position.x < 0)
            position.x = 0;
        if (position.y + layout.height > content.height)
            position.y = content.height - layout.height;
        if (position.y < 0)
            position.y = 0;
        layout.x = position.x;
        layout.y = position.y;
    }, { {parent, Property::SIZE}, {this, Property::SIZE} });
}


void LayoutNode::size_inherited(LayoutNode* parent) {
    property_inherit(parent, Property::SIZE);
}

void LayoutNode::size_include(LayoutNode* child) {
    property_include(child, Property::SIZE);
}

void LayoutNode::size_fraction(LayoutNode* parent, float parent_width_fraction, float parent_height_fraction) {
    add_rule(Property::SIZE, [parent, parent_width_fraction, parent_height_fraction](Layout& properties) {
        auto content = parent->layout.get_content_rect();
        properties.width = content.width * parent_width_fraction;
        properties.height = content.height * parent_height_fraction;
    }, { {parent, Property::SIZE} });
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
        child->draw_hierarchy(frame, transform, window);
    }
}

 bool Widget::event_filter(const glm::vec2& position_transform, const sf::Event& event) {
     if (m_delegate_all_events)
         return true;
     auto pos = sf::Mouse().getPosition();
     return layout.contains(position_transform, pos.x, pos.y);
 }

bool Widget::event_hierarchy(const glm::vec2& position_transform, const sf::Event& event) {
    if (!event_filter(position_transform, event))
        return false;
    glm::vec2 transform = position_transform + glm::vec2{ layout.x + layout.padding.left, layout.y + layout.padding.top };
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it){
        if ((*it)->event_hierarchy(transform, event))
            return true;
    }
    return on_event ? on_event(transform, event) : true;
}

Widget* Widget::get_hovered(const glm::vec2& position_transform, glm::uvec2 mouse_pos) {
    glm::vec2 transform = position_transform + glm::vec2{ layout.x + layout.padding.left, layout.y + layout.padding.top };
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        Widget* w = (*it)->get_hovered(transform, mouse_pos);
        if (!w) return w;
    }
    if (layout.contains(position_transform, mouse_pos.x, mouse_pos.y))
        return this;
}

Widget* Widget::add(std::unique_ptr<Widget>&& child) {
    child->m_parent = this;
    m_children.push_back(std::move(child));
    return m_children.back().get();
}

void Widget::delete_widget(Widget* widget) {
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == widget) {
            m_children.erase(it);
            return;
        }
    }
}

Panel::Panel(sf::Color background_color, sf::Color border_color, float border) {
    m_rect.setFillColor(background_color);
    m_rect.setOutlineColor(border_color);
    m_rect.setOutlineThickness(-border);
    layout.padding = { border, border, border, border };
    m_delegate_all_events = false;
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
