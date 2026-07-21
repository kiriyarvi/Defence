#pragma once
#include "texture_manager.h"
#include "gui/widget.h"


class LayeredIcon: virtual public Widget {
public:
    LayeredIcon() = default;
    std::vector<TextureID> layers;
    bool grayscale = false;
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
protected:
    sf::Sprite m_sprite;
};
