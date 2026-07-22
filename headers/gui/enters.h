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
    void on_event(const sf::RenderWindow& window, sf::Event event);
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
