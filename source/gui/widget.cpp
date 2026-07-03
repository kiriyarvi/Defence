#include "gui/widget.h"

int LayoutNode::Dependency::Size = 0x01;
int LayoutNode::Dependency::Position = 0x10;

LayoutNode::Rect LayoutNode::Layout::get_content_rect() const {
    Rect r;
    r.x = layout_rect.x + margin.left + padding.left;
    r.y = layout_rect.y + margin.top + padding.top;
    r.height = layout_rect.height - margin.top - margin.bottom - padding.top - padding.bottom;
    r.width = layout_rect.width - margin.left - margin.right - padding.left - padding.right;
    return r;
}

LayoutNode::Rect LayoutNode::Layout::get_border_rect() const {
    Rect r;
    r.x = layout_rect.x + margin.left;
    r.y = layout_rect.y + margin.top ;
    r.height = layout_rect.height - margin.top - margin.bottom;
    r.width = layout_rect.width - margin.left - margin.right;
    return r;
}

glm::vec2 LayoutNode::Layout::layout_size(const glm::vec2& content_size) const {
    return {
        content_size.x + margin.left + margin.right + padding.left + padding.right,
        content_size.y + margin.top + margin.bottom + padding.top + padding.bottom
    };
}

void LayoutNode::calc_position(uint32_t update_frame) {
    if (m_last_position_update == update_frame)
        return; //уже вычисляли на данном frame => пропускаем
    for (auto& d : position_dependencies) {
        if (d.type & Dependency::Size)
            d.layout_node->calc_size(update_frame);
        if (d.type & Dependency::Position)
            d.layout_node->calc_position(update_frame);
    }
    if (position_function) {
        glm::vec2 pos = position_function();
        layout.layout_rect.x = pos.x;
        layout.layout_rect.y = pos.y;
    }
    m_last_position_update = update_frame;
}

void LayoutNode::calc_size(uint32_t update_frame) {
    if (m_last_size_update == update_frame)
        return; //уже вычисляли на данном frame => пропускаем
    for (auto& d : size_dependencies) {
        if (d.type & Dependency::Size)
            d.layout_node->calc_size(update_frame);
        if (d.type & Dependency::Position)
            d.layout_node->calc_position(update_frame);
    }
    if (size_function) {
        glm::vec2 size = size_function();
        layout.layout_rect.width = size.x;
        layout.layout_rect.height = size.y;
    }
    m_last_size_update = update_frame;
}

void LayoutNode::add_dependent(int that, LayoutNode* depentent, int relation) {
    Dependency d{ relation, depentent };
    if (that & Dependency::Position)
        position_dependencies.push_back(d);
    if (that & Dependency::Size)
        size_dependencies.push_back(d);
}

void LayoutNode::size_options_reset() {
    size_dependencies.clear();
    size_function = {};
}

void LayoutNode::position_options_reset() {
    position_dependencies.clear();
    position_function = {};
}


void LayoutNode::vbox(const std::vector<LayoutNode*>& elements) {
    for (auto& e : elements)
        add_dependent(Dependency::Size, e, Dependency::Size);
    size_function = [this, elements]() {
        glm::vec2 size = {0,0};
        for (auto& e : elements) {
            size.x = std::max(e->layout.layout_rect.width, size.x);
            size.y += e->layout.layout_rect.height;
        }
        return layout.layout_size(size);
    };
    for (size_t i = 1; i < elements.size(); ++i) {
        elements[i]->add_dependent(Dependency::Position, elements[i - 1], Dependency::Position & Dependency::Size);
        elements[i]->position_function = [prev = elements[i - 1]]()->glm::vec2 {
            return { prev->layout.layout_rect.x, prev->layout.layout_rect.y + prev->layout.layout_rect.height };
        };
    }
}

void LayoutNode::size_fixed(float width, float height) {
    size_options_reset();
    this->size_function = [width, height]() {
        return glm::vec2(width, height);
    };
}


void LayoutNode::position_centering(LayoutNode* parent) {
    position_options_reset();
    add_dependent(Dependency::Position, parent, Dependency::Size);
    add_dependent(Dependency::Position, this, Dependency::Size);
    this->position_function = [parent, this]() {
        return glm::vec2{ (parent->layout.get_content_rect().width - this->layout.layout_rect.width) / 2., (parent->layout.get_content_rect().height - this->layout.layout_rect.height) / 2};
    };
}

void LayoutNode::size_inherited(LayoutNode* parent) {
    size_options_reset();
    add_dependent(Dependency::Size, parent, Dependency::Size);
    this->size_function = [parent]()->glm::vec2 {
        auto content = parent->layout.get_content_rect();
        return { content.width, content.height };
    };
}

//LayoutNode оборачивает собой child
void LayoutNode::size_include(LayoutNode* child) {
    size_options_reset();
    add_dependent(Dependency::Size, child, Dependency::Size);
    this->size_function = [child, this]()->glm::vec2 {
        auto size = layout.layout_size({ child->layout.layout_rect.width, child->layout.layout_rect.height });
        return { size.x, size.y };
    };
}

void LayoutNode::size_fraction(LayoutNode* parent, float parent_width_fraction, float parent_height_fraction) {
    size_options_reset();
    add_dependent(Dependency::Size, parent, Dependency::Size);
    this->size_function = [=]()->glm::vec2 {
        auto content = parent->layout.get_content_rect();
        return { content.width * parent_width_fraction, content.height * parent_height_fraction };
    };
}

std::unique_ptr<Widget> Widget::create(Widget* parent) {
    return std::make_unique<Widget>(parent);
}

void Widget::draw_hierarchy(int frame, const glm::vec2& position_transform, sf::RenderWindow& window) {
    calc_size(frame);
    calc_position(frame);
    draw(position_transform, window);
    glm::vec2 transform = position_transform + glm::vec2{layout.layout_rect.x + layout.margin.left + layout.padding.left, layout.layout_rect.y + layout.margin.top + layout.padding.top };
    for (auto& child : m_children) {
        child->draw_hierarchy(frame, transform, window);
    }
}

Widget* Widget::add(std::unique_ptr<Widget>&& child) {
    child->m_parent = this;
    m_children.push_back(std::move(child));
    return m_children.back().get();
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

void Panel::draw(const glm::vec2& position_transform, sf::RenderWindow& window) {
    Rect widget_rect = layout.get_border_rect();
    m_rect.setSize({ widget_rect.width, widget_rect.height });
    m_rect.setPosition({ position_transform.x + widget_rect.x, position_transform.y + widget_rect.y });
    window.draw(m_rect);
}
