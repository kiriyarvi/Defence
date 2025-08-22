#include <SFML/Graphics.hpp>

#include "camera.h"
#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "game_state.h"

int main() {

	//sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Defence", sf::Style::Fullscreen);
	sf::RenderWindow window(sf::VideoMode(1000, 800), "Defence");

	auto& game_state = GameState::Instance(&window);
	auto& gui = GameState::Instance().get_tgui();

	Camera camera(window);

	sf::Clock clock;
	
	sf::Clock spawn_clock;

	EnemyManager::Instance().spawn();
	TileMap::Instance().build_guns();

	const float dt = 1.f / 60.f; // ������ ����������� 60 ��� � �������
	float accumulator = 0.f;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			if (gui.handleEvent(event));
			else if (camera.process(event));
		}
		// ������
		double dtime = clock.getElapsedTime().asMicroseconds();
		accumulator += dtime;
		if (accumulator > dt * 1000 * 1000) {
			dtime = accumulator;
			accumulator = 0;
			game_state.logic();
			if (!game_state.is_game_over()) {
				EnemyManager::Instance().logic(dtime);
				TileMap::Instance().logic(dtime);
				if (spawn_clock.getElapsedTime().asSeconds() > 2) {
					EnemyManager::Instance().spawn();
					spawn_clock.restart();
				}
				SoundManager::Instance().logic();
			}
		}
		//���������
		clock.restart(); // ������� ����� ����� ������.
		camera.apply(window);
		window.clear(sf::Color::Black);
		TileMap::Instance().draw(window);
		EnemyManager::Instance().draw(window);
		TileMap::Instance().draw_effects(window);
		gui.draw();
		window.display();
	}

	return 0;
}