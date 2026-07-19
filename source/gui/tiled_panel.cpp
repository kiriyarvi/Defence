#include "gui/tiled_panel.h"
#include "texture_manager.h"

/**
 * @brief 
 * @param type 
 * @param tile_size_reference виджет, высота которого задает размер тайла.
 */
TiledPanel::TiledPanel(Type type, Widget* tile_size_reference) :
    m_type{ type }, m_tile_size_reference{tile_size_reference}
{
    m_tile.setTexture(TextureManager::Instance().textures[TextureID::PanelsTileset]);

    content_widget = add_widget(Widget::create());
    DEBUG_TAG(content_widget, "TiledPanel::content_widget")

    add_rule(Property::SIZE, [this](Layout& layout) {
        m_cached_tile_size = (int)m_tile_size_reference->layout.height;
        m_cached_tiles_count = {
            std::ceil((content_widget->layout.width + m_cached_tile_size) / m_cached_tile_size),
            std::ceil((content_widget->layout.height + m_cached_tile_size) / m_cached_tile_size)
        }; //сколько понадобиться тайлов, чтобы вместить content_widget
        layout.width = m_cached_tiles_count.x * m_cached_tile_size;
        layout.height = m_cached_tiles_count.y * m_cached_tile_size;
    }, { { content_widget, Property::SIZE }, {m_tile_size_reference, Property::SIZE } });

    content_widget->position_centering(this);
}

void TiledPanel::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    auto get_texture_rect = [](size_t n) {
        return sf::IntRect((n % 5) * 16, (n / 5) * 16, 16, 16);
    };
    m_tile.setScale(m_cached_tile_size / 16.f, m_cached_tile_size / 16.f);
    for (size_t x = 0; x < m_cached_tiles_count.x; ++x) for (size_t y = 0; y < m_cached_tiles_count.y; ++y) {
        if (m_type == Type::Paper) {
            if (x == 0 && y == 0)
                m_tile.setTextureRect(get_texture_rect(0));
            else if (x == m_cached_tiles_count.x - 1 && y == 0)
                m_tile.setTextureRect(get_texture_rect(1));
            else if (x == m_cached_tiles_count.x - 1 && y == m_cached_tiles_count.y - 1)
                m_tile.setTextureRect(get_texture_rect(3));
            else if (x == 0 && y == m_cached_tiles_count.y - 1)
                m_tile.setTextureRect(get_texture_rect(2));
            else if ((x % 2 == 0) && y == 0)
                m_tile.setTextureRect(get_texture_rect(4));
            else if ((x % 2 != 0) && y == 0)
                m_tile.setTextureRect(get_texture_rect(5));
            else if (x == m_cached_tiles_count.x - 1 && (y % 2 == 0))
                m_tile.setTextureRect(get_texture_rect(6));
            else if (x == m_cached_tiles_count.x - 1 && (y % 2 == 1))
                m_tile.setTextureRect(get_texture_rect(7));
            else if ((x % 2 != 0) && y == m_cached_tiles_count.y - 1)
                m_tile.setTextureRect(get_texture_rect(8));
            else if ((x % 2 == 0) && y == m_cached_tiles_count.y - 1)
                m_tile.setTextureRect(get_texture_rect(9));
            else if (x == 0 && (y % 2 == 0))
                m_tile.setTextureRect(get_texture_rect(10));
            else if (x == 0 && (y % 2 == 1))
                m_tile.setTextureRect(get_texture_rect(11));
            else
                m_tile.setTextureRect(get_texture_rect(12));
        }
        else {
            if (x == 0 && y == 0)
                m_tile.setTextureRect(get_texture_rect(13));
            else if (x == m_cached_tiles_count.x - 1 && y == 0)
                m_tile.setTextureRect(get_texture_rect(14));
            else if (x == m_cached_tiles_count.x - 1 && y == m_cached_tiles_count.y - 1)
                m_tile.setTextureRect(get_texture_rect(16));
            else if (x == 0 && y == m_cached_tiles_count.y - 1)
                m_tile.setTextureRect(get_texture_rect(15));
            else if (y == 0)
                m_tile.setTextureRect(get_texture_rect(17));
            else if (x == m_cached_tiles_count.x - 1)
                m_tile.setTextureRect(get_texture_rect(18));
            else if (y == m_cached_tiles_count.y - 1)
                m_tile.setTextureRect(get_texture_rect(19));
            else if (x == 0)
                m_tile.setTextureRect(get_texture_rect(20));
            else
                m_tile.setTextureRect(get_texture_rect(21));
        }
        m_tile.setPosition( //нецелая позиция приводит к артефактам
            static_cast<int>(position_transform.x + layout.x + static_cast<float>(x) * m_cached_tile_size),
            static_cast<int>(position_transform.y + layout.y + static_cast<float>(y) * m_cached_tile_size)
        );
        window.draw(m_tile);
    }
}
