#pragma once
#include "gui/console.h"
#include "gui/enters.h"
#include "gui/building_buttons.h"
#include "gui/upgrade_panel.h"
#include "gui/next_wave_button.h"
#include "camera.h"

class GameState;

class GameUI {
public:
    GameUI(GameState& game_state, sf::RenderWindow& window);
    void create();

    void update_player_health(int health);
    void update_player_coins(int coins);
    void update_wave_info(bool wave_started);
    void update_on_enemy_defeated(int coins);
    void update_wave_indicator_text(const std::string& text);
    void game_over(bool win);
    void close_upgrade_panel();

    Camera& get_camera() { return m_camera; }
    Widget* get_tile_size_reference() { return m_tile_size_reference; }
    Console* get_console() { return m_console; }
    EntersWidget* get_enters_widget() { return m_enters_widget.get(); }

    void on_event(sf::Event& event);
    void logic(float dtime_mc);
    void draw_on_map_effects();
private:
    void on_button_pressed(ClickableWidget::Button::Type type);
private:
    GameState& m_game_state;
    sf::RenderWindow& m_render_window;
    Camera m_camera;
    HoverableClickableWidget* m_game_process_ui; // UI во время процесса игры
    Label* m_player_coins_count_widget;
    Label* m_player_health_count_widget;
    Label* m_wave_info;
    BuildingPanel* m_building_panel;
    NextWaveButton* m_next_wave_button;
    UpgradePanel* m_upgrade_panel = nullptr;
    Widget* m_tile_size_reference;
    Widget* m_upgrade_panel_height_reference;
    Console* m_console;
    std::unique_ptr<EntersWidget> m_enters_widget = nullptr;
};

