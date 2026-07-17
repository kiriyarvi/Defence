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
    m_scroll_indicator.setTextureRect(sf::IntRect(53, 2, 3, 12));

    add_rule(Property::SIZE, [sg = m_scroll_groove](Layout& layout) {
        layout.width = 3 / 5.f * sg->layout.width;
        layout.height = 12 / 5.f * sg->layout.width;
    }, { { m_scroll_groove, Property::WIDTH } });
    add_rule(Property::POSITION, [this](Layout& layout) {
        float groove_space = m_scroll_groove->layout.height - m_scroll_groove->layout.width * (2 / 5.f);
        layout.y = m_scroll_groove->get_scroll() * (groove_space - layout.height) + m_scroll_groove->layout.width * (1 / 5.f);
        layout.x = m_scroll_groove->layout.width * (1 / 5.f);
    }, { {this, Property::HEIGHT}, {m_scroll_groove, Property::SIZE} });
}

void ScrollIndicator::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    m_scroll_indicator.setScale(layout.width / 3, layout.height / 12);
    m_scroll_indicator.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    window.draw(m_scroll_indicator);
}

ScrollIndicatorGroove::ScrollIndicatorGroove(Direction direction): m_direction(direction) {
    m_scroll_indicator_groove.setTexture(TextureManager::Instance().textures[TextureID::Slider]);
    Widget* scroll_indicator = add_widget(std::make_unique<ScrollIndicator>(m_direction, this));
    DEBUG_TAG(scroll_indicator, "scroll_indicator");
}

void ScrollIndicatorGroove::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    //Рисуем борозду.
    m_scroll_indicator_groove.setTextureRect(sf::IntRect(56, 2, 5, 13));
    m_scroll_indicator_groove.setScale(layout.width / 5.f, layout.height / 13.f);
    m_scroll_indicator_groove.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    window.draw(m_scroll_indicator_groove);

    m_scroll_indicator_groove.setTextureRect(sf::IntRect(56, 0, 5, 1));
    m_scroll_indicator_groove.setScale(layout.width / 5.f, layout.width / 5.f);
    window.draw(m_scroll_indicator_groove);
    m_scroll_indicator_groove.move(0, layout.height - layout.width / 5.f);
    window.draw(m_scroll_indicator_groove);
}


OneDirectionalScroller::OneDirectionalScroller(Direction direction) : m_direction{direction} {
    //Hierarchy
    m_blocker = (Blocker*)add_widget(std::make_unique<Blocker>());
    DEBUG_TAG(m_blocker, "blocker");
    m_content_widget = m_blocker->add_widget(Widget::create());
    DEBUG_TAG(m_content_widget, "m_content_widget");
    m_scroller_grove_widget = (ScrollIndicatorGroove*)add_widget(std::make_unique<ScrollIndicatorGroove>(direction));
    DEBUG_TAG(m_scroller_grove_widget, "m_scroller_grove_widget");
    //Layout
    m_scroller_grove_widget->property_inherit(m_blocker, Property::HEIGHT); //m_scroller_grove_widget наследует HEIGHT у m_blocker.
    m_scroller_grove_widget->position_anchor(Anchor::TOP | Anchor::LEFT, m_blocker, Anchor::TOP | Anchor::RIGHT);  // расположен строго справа от m_blocker
    //WIDTH указывает пользователь
    

    add_rule(Property::SIZE, [this](Layout& layout) {
        layout.width = m_blocker->layout.width + m_scroller_grove_widget->layout.width;
        layout.height = m_blocker->layout.height;
    }, { { m_blocker, Property::SIZE }, {m_scroller_grove_widget, Property::HEIGHT} });
}

