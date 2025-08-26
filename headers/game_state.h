#pragma once
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

#include "tile_map.h"
#include <functional>
#include "building_buttons.h"
#include "enemies/IEnemy.h"

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
	
	tgui::Gui& get_tgui();

	bool event(sf::Event& event, const sf::RenderWindow& current_window);
	void logic();
	void draw(sf::RenderWindow& current_window);
	bool is_player_defeated() { return  m_player_hp <= 0; }
    bool is_game_over() { return is_player_defeated() || m_win; }
	void player_health_add(int health);
	void player_coins_add(int coins);
    void enemy_defeated(EnemyType type);
    void win();
	//void minigun_state_update(const MiniGun& minigun);
private:
	friend class BuildingButton;	
private:
	GameState(sf::RenderWindow& window);
private:
	int m_player_hp = 10;
	int m_player_coins = 0;
private:
    bool m_win = false;
	tgui::Gui m_gui;
	tgui::Label::Ptr m_player_health_count_widget;
	tgui::Label::Ptr m_player_coins_count_widget;
	tgui::Label::Ptr m_centered_message; // сообщение по центру

	BuildingButton* m_current_building_construction = nullptr;
	sf::Vector2f m_mouse_pos;
	std::list<std::unique_ptr<BuildingButton>> m_building_buttons;
public:
    tgui::Font GOSTtypeA_font;
    tgui::Font PixelSplitter_Bold_font; // расположены здесь, чтобы уничтожались первее (иначе ошибка в tgui).
};
