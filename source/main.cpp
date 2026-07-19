#include <SFML/Graphics.hpp>

#include "camera.h"
#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "game_state.h"
#include "animation_holder.h"
#include "debugger.h"
#include "net_manager.h"

#if 1

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
#include "gui/tiled_panel.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Defence");
    GUI::Instance().set_root(Widget::create(), window);
    Widget* root = GUI::Instance().get_root();
    DEBUG_TAG(root, "root");

  

    Widget* tile_size = root->add_widget(Widget::create());
    tile_size->property_equal(Property::HEIGHT, false, root, Property::HEIGHT, false, modifiers::Multiply(0.05));
    tile_size->property_equal(Property::WIDTH, false, root, Property::HEIGHT, false, modifiers::Multiply(0.05));

    //HIERARCHY
    //Tiled panel
    TiledPanel* tiled_panel = (TiledPanel*)root->add_widget(std::make_unique<TiledPanel>(TiledPanel::Type::Paper, tile_size));
    DEBUG_TAG(tiled_panel, "tiled_panel");
        //Content widget
        Widget* tiled_panel_content_widget = tiled_panel->content_widget;
        DEBUG_TAG(tiled_panel_content_widget, "tiled_panel_content_widget");
            //vertical groove
            ScrollIndicatorGroove* groove_vertical = (ScrollIndicatorGroove*)tiled_panel_content_widget->add_widget(std::make_unique<ScrollIndicatorGroove>(Direction::VERTICAL, ScrollIndicatorType::Paper));
            DEBUG_TAG(groove_vertical, "groove_vertical");
            groove_vertical->property_equal(Property::WIDTH, false, root, Property::HEIGHT, true, modifiers::Multiply(0.05));
            //scrollable_panel, groove_vertical controls scroll
            ScrollablePanel* scrollable_panel = (ScrollablePanel*)tiled_panel_content_widget->add_widget(std::make_unique<ScrollablePanel>(Direction::VERTICAL, groove_vertical));
            DEBUG_TAG(scrollable_panel, "scrollable_panel");
                //scrollable panel content
                Widget* scrollable_panel_content = scrollable_panel->content_widget;
                DEBUG_TAG(scrollable_panel_content, "scrollable_panel_content");
                    //Panel
                    Panel* panel = (Panel*)scrollable_panel_content->add_widget(Panel::create(sf::Color::Transparent, sf::Color(56, 42, 28)));
                    DEBUG_TAG(panel, "panel")
                        //Label on panel
                        Label* label = (Label*)panel->add_widget(Label::create());
                        DEBUG_TAG(label, "label")
                        for (size_t i = 0; i < 10; ++i)
                            label->add_text("This is the example of TiledPanel with paper style, you can check that all sizes are computed correctly and ensure this just seing the source code.", sf::Color(56, 42, 28));
    //LAYOUT
    label->property_from_content(panel, Property::WIDTH);
    panel->property_content_from(label, Property::HEIGHT);
    panel->property_equal(Property::WIDTH, false, root, Property::WIDTH, false, modifiers::Multiply(0.5));

    scrollable_panel_content->size_include(panel);

    scrollable_panel->property_equal(Property::HEIGHT, false, root, Property::HEIGHT, false, modifiers::Multiply(0.5));
    scrollable_panel->property_content_from(scrollable_panel_content, Property::WIDTH);

    groove_vertical->property_equal(Property::HEIGHT, false, scrollable_panel, Property::HEIGHT, false, {});
    groove_vertical->property_equal(Property::WIDTH, false, scrollable_panel, Property::HEIGHT, false, modifiers::Multiply(0.05));
    tiled_panel_content_widget->hbox({ scrollable_panel , groove_vertical });
    tiled_panel->position_centering();

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
