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


	const float dt = 1.f / 60.f; // логика обновляется 60 раз в секунду
	float accumulator = 0.f;

	int wave_number = 1;
	int wave_enemies_count = 4;
	int spawned = 0;
	double wave_delay = 5;
	bool wave = false;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			if (gui.handleEvent(event));
			else {
				if (!game_state.event(event, window))
					camera.process(event);
			}
		}
		// логика
		double dtime = clock.getElapsedTime().asMicroseconds();
		
		

		accumulator += dtime;
		if (accumulator > dt * 1000 * 1000) {
			dtime = accumulator;
			accumulator = 0;
			game_state.logic();
			if (!game_state.is_game_over()) {
				EnemyManager::Instance().logic(dtime);
				TileMap::Instance().logic(dtime);
				if (!wave) {
					if (spawn_clock.getElapsedTime().asSeconds() > wave_delay) {
						wave = true;
						spawned = 0;
						spawn_clock.restart();
					}
				}
				else {
					if (spawn_clock.getElapsedTime().asSeconds() > 10.f / wave_enemies_count) {
						EnemyManager::Instance().spawn();
						spawn_clock.restart();
						++spawned;
						if (spawned >= wave_enemies_count) {
							++wave_number;
							wave_enemies_count += wave_number;
							wave = false;
						}
					}
				}
				
				SoundManager::Instance().logic();
			}
		}
		//отрисовка
		clock.restart(); // рестарт часов после логики.
		camera.apply(window);
		window.clear(sf::Color::Black);
		TileMap::Instance().draw(window);
		EnemyManager::Instance().draw(window);
		TileMap::Instance().draw_effects(window);
		game_state.draw(window);
		gui.draw();
		window.display();
	}

	return 0;
}