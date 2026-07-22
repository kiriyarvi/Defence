#include "gui/tooltip.h"
#include "game_state.h"

std::pair<std::unique_ptr<Panel>, Label*> create_tooltip(Anchor::Type tooltip_anchor) {
    //создаем tooltip
    auto panel_ptr = Panel::create(sf::Color(50, 50, 50, 255), sf::Color::Black, 0);
    Panel* panel = panel_ptr.get();
    DEBUG_TAG(panel, "tooltip_panel")
        Label* label = (Label*)panel->add_widget(Label::create(true)); //можем вызвать add_widget, поскольку panel пока не вмонтирован в общую иерархию
    DEBUG_TAG(label, "tooltip_label")
        panel->size_include(label);
    panel->position_tooltip(tooltip_anchor);
    panel->hit_test_policy = Widget::HitTestPolicy::Terminate;
    return std::make_pair(std::move(panel_ptr), label);
}


std::pair<std::unique_ptr<TiledPanel>, Label*> create_tooltip_paper(Anchor::Type tooltip_anchor, Widget* tile_size_reference) {
    //создаем tooltip
    auto panel_ptr = std::make_unique<TiledPanel>(TiledPanel::Type::Paper, tile_size_reference ? tile_size_reference : GameState::Instance().get_ui().get_tile_size_reference());
    TiledPanel* panel = panel_ptr.get();
    DEBUG_TAG(panel, "tooltip_panel")
    Label* label = (Label*)panel->content_widget->add_widget(Label::create(true)); //можем вызвать add_widget, поскольку panel пока не вмонтирован в общую иерархию
    DEBUG_TAG(label, "tooltip_label")
    panel->content_widget->size_include(label);
    panel->position_tooltip(tooltip_anchor);
    panel->hit_test_policy = Widget::HitTestPolicy::Terminate;
    return std::make_pair(std::move(panel_ptr), label);
}

