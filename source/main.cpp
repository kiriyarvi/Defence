#include <SFML/Graphics.hpp>

#include "camera.h"
#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "game_state.h"
#include "animation_holder.h"

int main() {

	//sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Defence", sf::Style::Fullscreen);
	sf::RenderWindow window(sf::VideoMode(1000, 800), "Defence");

	auto& game_state = GameState::Instance(&window);
    game_state.init_stage(1);
	auto& gui = GameState::Instance().get_tgui();
    EnemyManager::Instance().init();

	Camera camera(window);

	sf::Clock clock;

	
	const float dt = 1.f / 60.f; // логика обновляется 60 раз в секунду
	float accumulator = 0.f;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
            if (!gui.handleEvent(event)) {
				camera.process(event);
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Key::Q) {
                    Tile::new_tiling_mode = !Tile::new_tiling_mode;
                }
            }
            game_state.event(event, window);
			
		}
		// логика
		double dtime = clock.getElapsedTime().asMicroseconds();
		accumulator += dtime;
		if (accumulator > dt * 1000 * 1000) {
			dtime = accumulator;
			accumulator = 0;
			game_state.logic(dtime);
			if (!game_state.is_game_over() && !game_state.is_help_displayed()) {
				EnemyManager::Instance().logic(dtime);
				TileMap::Instance().logic(dtime);
				SoundManager::Instance().logic();
                AnimationHolder::Instance().logic(dtime);
			}
		}
		//отрисовка
		clock.restart(); // рестарт часов после логики.
		camera.apply(window);
		window.clear(sf::Color::Black);
		TileMap::Instance().draw(window);
		EnemyManager::Instance().draw(window);
		TileMap::Instance().draw_effects(window);
        AnimationHolder::Instance().draw(window);
		game_state.draw(window);
		gui.draw();
		window.display();
	}

	return 0;
}
