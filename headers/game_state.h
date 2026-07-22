#pragma once
#include "tile_map.h"
#include <functional>
#include "enemies/IEnemy.h"
#include "utils/animation.h"
#include "game_ui.h"

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
    //GETTERS
    State get_state() const { return m_state; }
    int get_player_coins() const { return m_player_coins; }
    int get_player_health() const { return m_player_hp; }
    float get_time_multiplier() const { return m_time_multiplier; }
    GameUI& get_ui() { return m_ui; }
    //SETTERS
	void player_health_add(int health);
    void kill_player();
	void player_coins_add(int coins);
    void set_time_multiplier(float multiplier) { m_time_multiplier = multiplier; }
    //EVENTS
    void enemy_defeated(EnemyType type);
    void win();
    void wave(bool started);
    void start_game();
    //TRIPLE
    void on_event(sf::Event& event);
	void logic(double dtime_mc);
	void draw(sf::RenderWindow& current_window);
private:
	friend class BuildingButton;	
private:
	GameState(sf::RenderWindow& window);
    ~GameState();
    void init_stage(int stage);
private:
    GameUI m_ui;
	int m_player_hp = 10;
	int m_player_coins = 0;
    float m_time_multiplier = 1.f;
    State m_state = State::PREPAIRING;
    bool m_win = false;
    void set_game_finished_state(bool win);
};
