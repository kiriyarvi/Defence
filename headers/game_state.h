#pragma once
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"


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

	void logic();
	bool is_game_over() { return  player_hp <= 0; }
	void player_health_add(int health);
private:
	GameState(sf::RenderWindow& window);
	tgui::Gui gui;
	int player_hp = 10;
	tgui::Label::Ptr player_health_count_widget;
	tgui::Label::Ptr centered_message; // сообщение по центру
};