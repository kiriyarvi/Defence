#pragma once
#include "tile_map.h"
#include <functional>
#include "gui/building_buttons.h"
#include "enemies/IEnemy.h"
#include "gui/help.h"
#include "gui/upgrade_panel.h"
#include "gui/widget.h"
#include "gui/label.h"
#include "gui/next_wave_button.h"
#include "gui/console.h"
#include "utils/animation.h"
#include "gui/enters.h"

#include <list>

class GameState {
public:
	static GameState& Instance(sf::RenderWindow* window = nullptr) {
		static GameState instance(*window);
		return instance;
	}
	// Удаляем копирование и перемещение
	GameState(const GameState&) = delete;
	GameState& operator=(const GameState&) = delete;
	GameState(GameState&&) = delete;
	GameState& operator=(GameState&&) = delete;

    enum class State {
        PREPAIRING, //< подготовка (игрок может перегенирировать карту или начать игру)
        GAME, //< игра
        GAME_FINISHED //< игра окончена (статус в m_win)
    };
    State get_state() const { return m_state; }

	bool event(sf::Event& event, const sf::RenderWindow& current_window);
	void logic(double dtime_mc);
	void draw(sf::RenderWindow& current_window);
	void player_health_add(int health);
    void kill_player();
	void player_coins_add(int coins);
    int get_player_coins() const { return m_player_coins; }
    void enemy_defeated(EnemyType type);
    void win();
    void init_stage(int stage);
    void set_wave_info(const std::string& wave);
    Console* get_console() { return m_console; }
    EntersWidget* get_enters_widget() { return m_enters_widget.get(); }
    void wave_preparing();
    void wave_started();
    float get_time_multiplier() const { return m_time_multiplier; }
    Widget* get_tile_size_reference() { return m_tile_size_reference; }
    void close_upgrade_panel();

    sf::Window& window;
private:
	friend class BuildingButton;	
private:
	GameState(sf::RenderWindow& window);
    ~GameState();
private:
	int m_player_hp = 10;
	int m_player_coins = 0;
private:
    Widget* m_game_process_ui; // UI во время процесса игры
    Label* m_player_coins_count_widget;
    Label* m_player_health_count_widget;
    Label* m_wave_info;
    BuildingPanel* m_building_panel;
    NextWaveButton* m_next_wave_button;
    float m_time_multiplier = 1.f;
    UpgradePanel* m_upgrade_panel = nullptr;
    Widget* m_tile_size_reference;
    Widget* m_upgrade_panel_height_reference;
    Console* m_console;
    State m_state = State::PREPAIRING;
    bool m_win = false;
    void set_game_finished_state(bool win);
    std::unique_ptr<EntersWidget> m_enters_widget = nullptr;

	sf::Vector2f m_mouse_pos;
};
