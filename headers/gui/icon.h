#pragma once
#include "gui/widget.h"
#include "texture_manager.h"
#include <SFML/Graphics.hpp>

class Icon : public Widget {
public:
    Icon(TextureID texture);
    static std::unique_ptr<Icon> create(TextureID texture);
    void draw(const glm::vec2& position_transform, sf::RenderWindow& window) override;
private:
    sf::Sprite m_sprite;
};
