#include <SFML/Graphics.hpp>

#include "camera.h"
#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "game_state.h"
#include "animation_holder.h"
#include "debugger.h"
#include "net_manager.h"

#if 0

int main() {
	//sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Defence", sf::Style::Fullscreen);
	sf::RenderWindow window(sf::VideoMode(1000, 800), "Defence");
    GUI::Instance().set_root(Widget::create(),window);
	auto& game_state = GameState::Instance(&window);
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
            if (!GUI::Instance().event(event) && !gui.handleEvent(event)) {
                camera.process(event);
                game_state.event(event, window);
            }
		}
		// логика
		double dtime = clock.getElapsedTime().asMicroseconds();
		accumulator += dtime;
		if (accumulator > dt * 1000 * 1000) {
			dtime = accumulator;
			accumulator = 0;
			game_state.logic(dtime);
			if (!game_state.is_game_over() && !game_state.is_help_displayed()) {
                float time_multiplier = GameState::Instance().get_time_multiplier();
				EnemyManager::Instance().logic(time_multiplier * dtime);
				TileMap::Instance().logic(time_multiplier * dtime);
                NetManager::Instance().logic(time_multiplier * dtime);
				SoundManager::Instance().logic();
                AnimationHolder::Instance().logic(time_multiplier * dtime);
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
        GUI::Instance().draw(window);
        window.display();
	}

	return 0;
}

#else

#include "gui/scroller.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Defence");
    GUI::Instance().set_root(Widget::create(), window);
    Widget* root = GUI::Instance().get_root();
    DEBUG_TAG(root, "root");

    OneDirectionalScroller* on_directional_scroller = (OneDirectionalScroller*)root->add_widget(std::make_unique<OneDirectionalScroller>(Direction::HORISONTAL));
    DEBUG_TAG(on_directional_scroller, "on_directional_scroller");

    on_directional_scroller->position_centering();
    Widget* _frame = on_directional_scroller->get_frame_widget();
    _frame->size_fixed(200, 200);
    on_directional_scroller->get_scroller_grove_widget()->add_rule(Property::WIDTH, [_frame](Widget::Layout& layout) {
        layout.width = _frame->layout.width * 0.1;
    }, { { _frame , Property::WIDTH} });
    Widget* content = on_directional_scroller->get_content_widget();

    Panel* panel = (Panel*)content->add_widget(Panel::create(sf::Color(176, 149, 87),sf::Color(116, 94, 41)));
    DEBUG_TAG(panel, "panel");
    Label* label = (Label*)panel->add_widget(Label::create(true));
    label->add_text("This is the WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\n long\n long\n long\n long\n long\n long\n long\n long\n text\n another\n long\n long\n long\n long\n long\n long\n long\n long");
    DEBUG_TAG(label, "label");
    panel->size_include(label);
    content->size_include(panel);


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
            GUI::Instance().event(event);
        }
        // логика
        //отрисовка
        clock.restart(); 
        window.clear(sf::Color::Black);
        GUI::Instance().draw(window);
        window.display();
    }
    return 0;
}

#endif
