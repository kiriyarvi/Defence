#include "gui/switch.h"
#include "texture_manager.h"

Switch::Switch(bool initial_state, const std::function<void(bool)>& on_changed) : Clickable(this, sf::Mouse::Left)
{
    m_switch_sprite.setTexture(TextureManager::Instance().textures[TextureID::Switch]);
    m_state = !initial_state;
    set_state(initial_state);
    add_rule(Property::WIDTH, [this](Layout& layout) {
        layout.width = (static_cast<float>(m_switch_sprite.getTextureRect().width) / m_switch_sprite.getTextureRect().height) * layout.height;
    }, { { this, Property::HEIGHT } });
    on_pressed = [on_changed, this](EventContext context) -> Query {
        set_state(!m_state);
        on_changed(m_state);
        return Query{ Query::PROCESSED };
    };
}

void Switch::set_state(bool state) {
    if (m_state == state)
        return;
    if (state == true)
        m_switch_sprite.setTextureRect(sf::IntRect(0, 16, 32, 13));
    else
        m_switch_sprite.setTextureRect(sf::IntRect(0, 0, 32, 13));
    m_state = state;
}

Query Switch::on_event(EventContext context) {
    return Clickable::click_event(context);
}

void Switch::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    m_switch_sprite.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    m_switch_sprite.setScale(layout.width / m_switch_sprite.getTextureRect().width, layout.height / m_switch_sprite.getTextureRect().height);
    window.draw(m_switch_sprite);
}
