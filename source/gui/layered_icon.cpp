#include "gui/layered_icon.h"
#include "shader_manager.h"

void LayeredIcon::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    m_sprite.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    sf::RenderStates states;
    if (grayscale)
        states.shader = &ShaderManager::Instance().shaders[Shader::GrayScale];
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        sf::Texture& texture = TextureManager::Instance().textures[*it];
        auto texture_size = texture.getSize();
        m_sprite.setScale({ (float)layout.width / texture_size.x, (float)layout.height / texture_size.y });
        m_sprite.setTexture(texture, true);
        window.draw(m_sprite, states);
    }
}
