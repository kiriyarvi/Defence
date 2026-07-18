#include "gui/scroller.h"
#include "texture_manager.h"

Blocker::Blocker() {
    hit_test_policy = HitTestPolicy::Block;
}

void Blocker::draw_hierarchy(int frame, const glm::vec2& position_transform, sf::RenderTarget& window) {
    calc_layout();
    glm::vec2 transform = position_transform + glm::vec2{ layout.x + layout.padding.left, layout.y + layout.padding.top };

    auto content_rect = layout.get_content_rect();
    if (m_texture.getSize().x != content_rect.width || m_texture.getSize().y != content_rect.height)
        m_texture.create(content_rect.width, content_rect.height);
    m_texture.clear(sf::Color::Transparent);
    for (auto& child : m_children) {
        child->draw_hierarchy(frame, child->layout.absolute ? (-transform) : glm::vec2(0.f, 0.f), m_texture);
    }
    m_texture.display();
    m_children_sprite.setTexture(m_texture.getTexture());
    m_children_sprite.setPosition(transform.x, transform.y);
    m_children_sprite.setScale(content_rect.width / m_texture.getSize().x, content_rect.height / m_texture.getSize().y);
    window.draw(m_children_sprite);
}

ScrollIndicator::ScrollIndicator(Direction direction, ScrollIndicatorGroove* scroll_groove) :
    m_direction(direction), m_scroll_groove(scroll_groove) {
    m_scroll_indicator.setTexture(TextureManager::Instance().textures[TextureID::Slider]);
    glm::u8vec2 texture_pos = { 53, 0 };
    glm::u8vec2 texture_size = this->texture_size();
    m_scroll_indicator.setTextureRect(sf::IntRect(texture_pos.x, texture_pos.y, texture_size.x, texture_size.y));
    if (m_direction == Direction::HORISONTAL) {
        m_scroll_indicator.setRotation(90);
        m_scroll_indicator.setOrigin(0, texture_size.y);
    }

    add_rule(Property::SIZE, [sg = m_scroll_groove, texture_size](Layout& layout) {
        float k = sg->get_k(); //коэффициент масштабирования по x
        layout.*sg->get_width_property() = texture_size.x * k;
        layout.*sg->get_height_property() = texture_size.y * k;
    }, { { m_scroll_groove, m_direction == Direction::VERTICAL ? Property::WIDTH : Property::HEIGHT } });
    add_rule(Property::POSITION, [this](Layout& layout) {
        if (m_scroll_invalidated)
            compute_scroll_by_mouse_pos();
        auto groove_rect = m_scroll_groove->groove_local_rect();
        if (m_direction == Direction::VERTICAL) {
            layout.y = m_scroll * (groove_rect.height - layout.height) + groove_rect.top;
            layout.x = groove_rect.left;
        }
        else {
            layout.x = m_scroll * (groove_rect.width - layout.width) + groove_rect.left;
            layout.y = groove_rect.top;
        }
    }, { {this, m_direction == Direction::VERTICAL ? Property::HEIGHT : Property::WIDTH }, {m_scroll_groove, Property::SIZE} });
}

void ScrollIndicator::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    glm::u8vec2 texture_size = this->texture_size();
    m_scroll_indicator.setScale(layout.*m_scroll_groove->get_width_property() / (float)texture_size.x, layout.*m_scroll_groove->get_height_property() / (float)texture_size.y);
    m_scroll_indicator.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    window.draw(m_scroll_indicator);
}

void ScrollIndicator::compute_scroll_by_mouse_pos() {
    auto groove_rect = m_scroll_groove->groove_global_rect();
    glm::vec2 mouse_pos = GUI::Instance().mouse_pos;
    if (m_direction == Direction::VERTICAL) {
        float y_rel_to_groove = mouse_pos.y - m_capture_offset - groove_rect.top;
        m_scroll = std::clamp(y_rel_to_groove / (groove_rect.height - layout.height), 0.f, 1.f);
    }
    else {
        float x_rel_to_groove = mouse_pos.x - m_capture_offset - groove_rect.left;
        m_scroll = std::clamp(x_rel_to_groove / (groove_rect.width - layout.width), 0.f, 1.f);
    }
    m_scroll_invalidated = false;
}

Query ScrollIndicator::on_event(EventContext context) {
    if (context.event_type == Event::BUTTON_PRESSED && GUI::Instance().mouse_button == sf::Mouse::Left) {
        GUI::Instance().subscribe_deffered(this, Event::MOUSE_MOVED | Event::BUTTON_RELEASED);
        if (m_direction == Direction::VERTICAL)
            m_capture_offset =  GUI::Instance().mouse_pos.y - get_position_transform().y;
        else
            m_capture_offset = GUI::Instance().mouse_pos.x - get_position_transform().x;
        return Query{ Query::PROCESSED };
    }
    else if (context.event_type == Event::MOUSE_MOVED && context.from_subscribe) {
        request_to_calc_my_mouse_pos();
        return Query{ Query::PROCESSED };
    }
    else if (context.event_type == Event::BUTTON_RELEASED && GUI::Instance().mouse_button == sf::Mouse::Left && context.from_subscribe) {
        GUI::Instance().unsubscribe_deffered(this, Event::MOUSE_MOVED | Event::BUTTON_RELEASED);
        return Query{ Query::PROCESSED };
    }
    return Query::ignore(context.from_subscribe);
}

void ScrollIndicator::set_scroll(float scroll) {
    m_scroll = scroll;
    m_scroll_invalidated = false;
    invalidate(Property::POSITION);

}

void ScrollIndicator::request_to_calc_my_mouse_pos() {
    m_scroll_invalidated = true;
    invalidate(Property::POSITION);
}

sf::FloatRect ScrollIndicatorGroove::groove_local_rect() {
    sf::FloatRect r;
    glm::u8vec2 texture_size = this->texture_size();
    sf::Rect<uint8_t> texture_content_rect = this->texture_content_rect();
    float lw = layout.*get_width_property();
    float lh = layout.*get_height_property();
    float k = lw / texture_size.x;
    (m_direction == Direction::VERTICAL ? r.height : r.width) = lh - k * (texture_size.y - texture_content_rect.height);
    (m_direction == Direction::VERTICAL ? r.width : r.height) = lw - k * (texture_size.x - texture_content_rect.width);
    (m_direction == Direction::VERTICAL ? r.left : r.top) = k * texture_content_rect.left;
    (m_direction == Direction::VERTICAL ? r.top : r.left) = k * texture_content_rect.top;
    return r;
}

Property Widget::Layout::* ScrollIndicatorGroove::get_width_property() {
    return m_direction == Direction::VERTICAL ? &Layout::width : &Layout::height;
}

Property Widget::Layout::* ScrollIndicatorGroove::get_height_property() {
    return m_direction == Direction::VERTICAL ? &Layout::height : &Layout::width;
}

float ScrollIndicatorGroove::get_k() {
    return layout.*get_width_property() / texture_size().x;
}

sf::FloatRect ScrollIndicatorGroove::groove_global_rect() {
    auto local = groove_local_rect();
    glm::vec2 offset = get_position_transform();
    local.left += offset.x;
    local.top += offset.y;
    return local;
}


ScrollIndicatorGroove::ScrollIndicatorGroove(Direction direction, ScrollIndicatorType type):
    m_direction(direction), m_type(type) {
    m_scroll_indicator_groove.setTexture(TextureManager::Instance().textures[TextureID::Slider]);
    m_indicator = (ScrollIndicator*)add_widget(std::make_unique<ScrollIndicator>(m_direction, this));
    DEBUG_TAG(m_indicator, "scroll_indicator");
}

void ScrollIndicatorGroove::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    //Рисуем борозду.
    glm::u8vec2 groove_begin = m_type == ScrollIndicatorType::Paper ? glm::u8vec2{ 56,0 } : glm::u8vec2{ 61, 0 };
    sf::Rect<uint8_t> texture_content_rect = this->texture_content_rect();
    glm::u8vec2 groove_size = texture_size();
    if (m_direction == Direction::HORISONTAL) {
        m_scroll_indicator_groove.setRotation(90);
        m_scroll_indicator_groove.setOrigin(sf::Vector2f(0, texture_content_rect.height));
    }
    float k = get_k();
    m_scroll_indicator_groove.setTextureRect(sf::IntRect(groove_begin.x, groove_begin.y + texture_content_rect.top, groove_size.x, texture_content_rect.height));
    m_scroll_indicator_groove.setScale(k, layout.*get_height_property() / texture_content_rect.height);
    m_scroll_indicator_groove.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    window.draw(m_scroll_indicator_groove);

    m_scroll_indicator_groove.setScale(k, k);
    if (m_direction == Direction::HORISONTAL) {
        m_scroll_indicator_groove.setOrigin(sf::Vector2f(0, texture_content_rect.top));
    }
    m_scroll_indicator_groove.setTextureRect(sf::IntRect(groove_begin.x, groove_begin.y, groove_size.x, texture_content_rect.top));
    window.draw(m_scroll_indicator_groove);
    m_scroll_indicator_groove.setTextureRect(sf::IntRect(
        groove_begin.x,
        groove_begin.y + texture_content_rect.top + texture_content_rect.height,
        groove_size.x,
        groove_size.y - texture_content_rect.top - texture_content_rect.height
    ));
    if (m_direction == Direction::VERTICAL)
        m_scroll_indicator_groove.move(0, layout.height - (groove_size.y - texture_content_rect.top - texture_content_rect.height) * k);
    else
        m_scroll_indicator_groove.move(layout.width - (groove_size.y - texture_content_rect.top - texture_content_rect.height) * k, 0);
    window.draw(m_scroll_indicator_groove);
}


Query ScrollIndicatorGroove::on_event(EventContext context) {
    if (context.event_type == Event::BUTTON_PRESSED && GUI::Instance().mouse_button == sf::Mouse::Left) {
        m_indicator->request_to_calc_my_mouse_pos();
        return Query{ Query::PROCESSED };
    }
    return Query{};
}

