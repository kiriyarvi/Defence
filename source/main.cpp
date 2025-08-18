#include <SFML/Graphics.hpp>

#include "camera.h"
#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"

int main() {
	//sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Defence", sf::Style::Fullscreen);
	sf::RenderWindow window(sf::VideoMode(1000, 800), "Defence");
	Camera camera(window);

	sf::Clock clock;
	
	sf::Clock spawn_clock;

	EnemyManager::Instance().spawn();
	TileMap::Instance().build_guns();

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			if (camera.process(event));
		}
		// логика
		EnemyManager::Instance().logic(clock.getElapsedTime().asMicroseconds());
		TileMap::Instance().logic(clock.getElapsedTime().asMicroseconds());
		if (spawn_clock.getElapsedTime().asSeconds() > 2) {
			EnemyManager::Instance().spawn();
			spawn_clock.restart();
		}

		//отрисовка
		clock.restart(); // рестарт часов после логики.
		camera.apply(window);
		window.clear(sf::Color::Black);
		TileMap::Instance().draw(window);
		EnemyManager::Instance().draw(window);
		TileMap::Instance().draw_effects(window);
		SoundManager::Instance().logic();
		window.display();
	}

	return 0;
}