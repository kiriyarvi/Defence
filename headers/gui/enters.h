#pragma once
#include "tile_map.h"
#include "gui/widget.h"


class EntersWidget {
public:
    EntersWidget(Widget* ui);
    struct Enter {
        int x_id;
        int y_id;
        RoadGraph::PathID id;
        std::string content;
        RouteDrawer drawer;
    };
    void add_enter(RoadGraph::PathID id, const std::string& content);
    void delete_all_enters();
    void on_mouse_moved_on_map(const sf::Vector2f& mouse_pos);
    void on_mouse_leave_map(); //мышь покинула карту, например, она карта заслонена дургим виджетом.
    void draw(sf::RenderWindow& window);
    void logic(float dtime_mc);
private:
    void create_tooltip(const std::string& content);
    void delete_tooltip();
    sf::Sprite m_enter_sprite;
    std::vector<Enter> m_enters;
    Enter* m_hovered_enter = nullptr;
    Widget* m_ui;
    Widget* m_tooltip_widget = nullptr;
};
