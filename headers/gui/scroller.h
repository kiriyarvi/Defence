#pragma once

#include "gui/widget.h"

///Блокировщик. Ограничивает отрисовку своих детей. Будет видна только та часть детей, которая находится в content_rect этого виджета
///Также события будут поступать дочерним виджетам только в том случае, если они видны в content_rect.
class Blocker : public Widget {
public:
    Blocker();
    void draw_hierarchy(int frame, const glm::vec2& position_transform, sf::RenderTarget& window) override;
private:
    sf::RenderTexture m_texture;
    sf::Sprite m_children_sprite;
};

enum Direction {
    HORISONTAL,
    VERTICAL
};

class ScrollIndicatorGroove;

class ScrollIndicator : public Widget {
public:
    ScrollIndicator(Direction direction, ScrollIndicatorGroove* scroll_groove);
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
    Query on_event(EventContext context) override;
    glm::u8vec2 texture_size() { return glm::u8vec2{ 3, 6 }; }
    float get_scroll() const { return m_scroll; }
    void set_scroll(float scroll);
    void request_to_calc_my_mouse_pos();
    void set_on_scroll_changed(const std::function<void(float)>& on_scroll) { m_on_scroll_changed = on_scroll; }
private:
    void compute_scroll_by_mouse_pos();
    bool m_scroll_invalidated = false;
    float m_scroll = 0.0f;//величина прокрутки. Максимальное значение равно 1.f
    ScrollIndicatorGroove* m_scroll_groove;
    Direction m_direction;
    sf::Sprite m_scroll_indicator;
    float m_capture_offset = 0.f;
    std::function<void(float)> m_on_scroll_changed;
};

enum class ScrollIndicatorType {
    Paper,
    BluePrint
};

class ScrollIndicatorGroove : public Widget {
public:
    ScrollIndicatorGroove(Direction direction, ScrollIndicatorType type);
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
    sf::Rect<uint8_t> texture_content_rect() const { return sf::Rect<uint8_t>{1, 1, 3, 14}; };
    glm::u8vec2 texture_size() { return glm::u8vec2{ 5,16 }; }
    Query on_event(EventContext context) override;
    sf::FloatRect groove_local_rect();
    sf::FloatRect groove_global_rect();
    Property Layout::* get_width_property();
    Property Layout::* get_height_property();
    float get_k(); //коэффициент масшьабирования текстуры
    ScrollIndicator* get_indicator(){ return m_indicator; }
private:
    ScrollIndicator* m_indicator;
    Direction m_direction;
    ScrollIndicatorType m_type;
    sf::Sprite m_scroll_indicator_groove;
};


class ScrollablePanel : public Blocker {
public:
    ScrollablePanel(Direction direction, ScrollIndicatorGroove* associated_groove);
    Query on_event(EventContext event_context) override;
    Widget* content_widget;

private:
    Direction m_direction;
    ScrollIndicatorGroove* m_associated_groove;
    float m_scroll = 0.0f;
    Property Layout::* m_axis_size_prop_member;
    Property::Type m_axis_size_prop;
    Property Layout::* m_axis_coord_prop_member;
    Property::Type m_axis_coord_prop;
    float m_cached_content_size_prop;
    float m_cached_this_size_prop;
};


