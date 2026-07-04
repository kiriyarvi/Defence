#include "gui/icon.h"


Icon::Icon(TextureID texture) {
    m_sprite.setTexture(TextureManager::Instance().textures[texture]);
}

std::unique_ptr<Icon> Icon::create(TextureID texture) {
    return std::make_unique<Icon>(texture);
}

void Icon::draw(const glm::vec2& position_transform, sf::RenderWindow& window) {
    m_sprite.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    auto texture_rect = m_sprite.getTextureRect();
    m_sprite.setScale({ (float)layout.width/ texture_rect.getSize().x, (float)layout.height / texture_rect.getSize().y });
    window.draw(m_sprite);
}
