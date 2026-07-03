#include <SFML/Graphics.hpp>

#include "camera.h"
#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "game_state.h"
#include "animation_holder.h"
#include "debugger.h"
#include "net_manager.h"

int main() {
	//sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Defence", sf::Style::Fullscreen);
	sf::RenderWindow window(sf::VideoMode(1000, 800), "Defence");
	auto& game_state = GameState::Instance(&window);
    game_state.get_gui().size_fixed(window.getSize().x, window.getSize().y);
	auto& gui = GameState::Instance().get_tgui();

	Camera camera(window);

	sf::Clock clock;

	
	const float dt = 1.f / 60.f; // логика обновляется 60 раз в секунду
	float accumulator = 0.f;

    size_t frame = 0;

	while (window.isOpen()) {
        if (frame > 10000000) {
            frame = 0;
        }
        ++frame;

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
            if (!gui.handleEvent(event)) {
				camera.process(event);
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::E) {
                TileMap::Instance().enlarge_map();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::K) {
                for (auto& enemy : EnemyManager::Instance().m_enemies)
                    enemy->health = 0;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::T) {
                TileMap::Instance().create_tile_test_map();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
                sf::Vector2i mouse_screen_pos = sf::Mouse().getPosition();
                auto mouse_pos = window.mapPixelToCoords(mouse_screen_pos);
                EnemyManager::Instance().add_smoke(Smoke({ mouse_pos.x, mouse_pos.y }, 4., 20.));
            }
            if (event.type == sf::Event::Resized) {
                game_state.get_gui().size_fixed(window.getSize().x, window.getSize().y);
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
                NetManager::Instance().logic(dtime);
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
		TileMap::Instance().draw_effects(window); // анимации взрывов и выстрелов.
        AnimationHolder::Instance().draw(window);
        EnemyManager::Instance().draw_effects(window); // health + smoke + uncovering box
        Debugger::Instance().draw(window);
		game_state.draw(window);
		gui.draw();

        window.setView(sf::View(sf::FloatRect( 0,0 , window.getSize().x, window.getSize().y )));
        game_state.get_gui().draw_hierarchy(frame, { 0,0 }, window);
        camera.apply(window);

        window.display();
	}

	return 0;
}
