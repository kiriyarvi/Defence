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
private:
    ScrollIndicatorGroove* m_scroll_groove;
    Direction m_direction;
    sf::Sprite m_scroll_indicator;
};

class ScrollIndicatorGroove : public Widget {
public:
    ScrollIndicatorGroove(Direction direction);
    float get_scroll() const { return m_scroll; }
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
private:
    Direction m_direction;
    sf::Sprite m_scroll_indicator_groove;
    float m_scroll = 0.f; //величина прокрутки. Максимальное значение равно 1.f
};

class OneDirectionalScroller : public Widget {
public:
    OneDirectionalScroller(Direction direction);
    /// возвращает виджет контента, именно в него нужно помещать потомков! POSITION данного виджета полностью контролруется OneDirectionalScroller. Необходимо указать только SIZE
    Widget* get_content_widget() { return m_content_widget; }
    /// Окно, ограничивающее контент, укажите функцию вычисления его размеров.
    Widget* get_frame_widget() { return m_blocker; }
    ///Возвращает виджет канавки индикатора прокрути. POSITION контролируется OneDirectionalScroller. Укажите только WIDTH или HEIGHT в зависимости от Direction
    Widget* get_scroller_grove_widget() { return m_scroller_grove_widget; }
private:
    Widget* m_blocker;
    Widget* m_content_widget;
    ScrollIndicatorGroove* m_scroller_grove_widget;
    Direction m_direction;
};
