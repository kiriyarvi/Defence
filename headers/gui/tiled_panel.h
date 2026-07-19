#pragma once
#include "gui/widget.h"

class TiledPanel : public Widget {
public:
    enum class Type {
        Paper,
        Blueprint
    };
    TiledPanel(Type type, Widget* tile_size_reference);
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window);
    Widget* content_widget;
protected:
    Type m_type;
    sf::Sprite m_tile;
    Widget* m_tile_size_reference;
    float m_cached_tile_size = 0.0f;
    glm::uvec2 m_cached_tiles_count;
};
