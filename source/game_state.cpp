#include "game_state.h"

#include "guns/mine.h"
#include "guns/minigun.h"
#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "achievement_system.h"
#include "enemy_manager.h"
#include "resource_manager.h"

#include "gui/label.h"
#include "gui/icon.h"
#include "gui/scale.h"

GameState::GameState(sf::RenderWindow& window) : m_gui(window), window{window} {
    Widget* root = GUI::Instance().get_root();
    DEBUG_TAG(root, "root")
    m_game_process_ui = root->add_widget(Widget::create());
    m_game_process_ui->size_inherited(root);
    DEBUG_TAG(m_game_process_ui, "m_game_process_ui")

    //coin indicator (Hierarchy)
    m_player_coins_count_widget = (Label*)m_game_process_ui->add_widget(Label::create(true, 36, &ResourceManager::Instance().PixelSplitter_Bold_font));
    DEBUG_TAG(m_player_coins_count_widget, "m_player_coins_count_widget")
    m_player_coins_count_widget->add_text(std::to_string(m_player_coins), Label::coins_color);
    Icon* coin_icon = (Icon*)m_game_process_ui->add_widget(Icon::create(TextureID::Coin));
    DEBUG_TAG(coin_icon, "coin_icon")
    //coin indicator (Layout)
    coin_icon->add_rule(Property::SIZE, [coins_counter = m_player_coins_count_widget](Widget::Layout& layout) {
        layout.width = coins_counter->layout.height;
        layout.height = coins_counter->layout.height;
    }, { {m_player_coins_count_widget, Property::SIZE} });
    coin_icon->position_anchor(Anchor::LEFT, m_player_coins_count_widget, Anchor::RIGHT);
    //health indicator (Hierarchy)
    m_player_health_count_widget = (Label*)m_game_process_ui->add_widget(Label::create(true, 36, &ResourceManager::Instance().PixelSplitter_Bold_font));
    DEBUG_TAG(m_player_health_count_widget, "m_player_health_count_widget")
    m_player_health_count_widget->add_text("X" + std::to_string(m_player_hp));
    Icon* heart_icon = (Icon*)m_game_process_ui->add_widget(Icon::create(TextureID::Heart));
    DEBUG_TAG(heart_icon, "heart_icon")
    //health indicator (Layout)
    m_player_health_count_widget->position_anchor(Anchor::RIGHT | Anchor::TOP, m_game_process_ui, Anchor::RIGHT | Anchor::TOP);
    heart_icon->add_rule(Property::SIZE, [hp_counter = m_player_health_count_widget](Widget::Layout& layout) {
        layout.width = hp_counter->layout.height;
        layout.height = hp_counter->layout.height;
    }, { {m_player_health_count_widget, Property::SIZE} });
    heart_icon->position_anchor(Anchor::RIGHT, m_player_health_count_widget, Anchor::LEFT);

    //wave info (Hierarchy)
    m_wave_info = (Label*)m_game_process_ui->add_widget(Label::create(true, 36, &ResourceManager::Instance().PixelSplitter_Bold_font));
    DEBUG_TAG(m_wave_info, "m_wave_info")
    //wave info (Layout)
    m_wave_info->position_anchor(Anchor::TOP, m_game_process_ui, Anchor::TOP);

    //building panel
    m_building_panel = (BuildingPanel*)m_game_process_ui->add_widget(std::make_unique<BuildingPanel>(m_game_process_ui));
    DEBUG_TAG(m_building_panel, "m_building_panel");

    //next wave button(Hierarchy)
    m_next_wave_button = m_game_process_ui->add_widget(std::make_unique<NextWaveButton>());
    DEBUG_TAG(m_next_wave_button, "m_next_wave_button");
    //next wave button(Layout)
    m_next_wave_button->position_anchor(Anchor::BOTTOM | Anchor::RIGHT, m_game_process_ui, Anchor::BOTTOM | Anchor::RIGHT);
    m_next_wave_button->add_rule(Property::SIZE, [ui = m_game_process_ui](Widget::Layout& layout) {
        layout.height = ui->layout.height * 0.1;
        layout.width = layout.height;
    }, { { m_game_process_ui, Property::HEIGHT } });

    //scale for speed control (Hierarchy)
    Scale* speed_controller = (Scale*)m_game_process_ui->add_widget(std::make_unique<Scale>());
    speed_controller->set_on_pos_update_callback([this](size_t pos) {
        m_time_multiplier = 1 + pos;
    });
    DEBUG_TAG(speed_controller, "speed_controller");
    //scale for speed control (Layout)
    speed_controller->position_anchor(Anchor::RIGHT | Anchor::BOTTOM, m_next_wave_button, Anchor::LEFT | Anchor::BOTTOM);
    speed_controller->property_from_content(m_game_process_ui, Property::HEIGHT, [](float h) {return  0.1 * h; });

    m_building_panel->add_rule(Property::WIDTH, [speed_controller](Widget::Layout& layout) {
        layout.width = speed_controller->layout.x;
    }, { {speed_controller, Property::X} });

    //m_tile_size_reference (испольщуется для тайловых виджетов)
    m_tile_size_reference = m_game_process_ui->add_widget(Widget::create());
    m_tile_size_reference->add_rule(Property::SIZE, [ui = m_game_process_ui](Widget::Layout& layout) {
        layout.width = ui->layout.width * 0.05;
        layout.height = ui->layout.width * 0.05;
    }, { { m_game_process_ui, Property::HEIGHT } } );
    //m_upgrade_panel_height_reference (вспомогательный виджет для панели апгрейдов), вычисляет высоту панели апгрейдов
    m_upgrade_panel_height_reference = m_game_process_ui->add_widget(Widget::create());
    m_upgrade_panel_height_reference->add_rule(Property::SIZE, [hc = m_player_health_count_widget, bp = m_building_panel, ui = m_game_process_ui](Widget::Layout& layout) {
        layout.height = ui->layout.height - hc->layout.height - bp->layout.height;
    }, { { m_player_health_count_widget, Property::HEIGHT }, {m_building_panel, Property::HEIGHT}, {m_game_process_ui, Property::HEIGHT} });

    //Console
    m_console = m_game_process_ui->add_widget(std::make_unique<Console>());
    m_console->position_anchor(Anchor::BOTTOM | Anchor::LEFT, m_building_panel, Anchor::TOP | Anchor::LEFT);

    GOSTtypeA_font = tgui::Font{ "fonts/GOSTtypeA.ttf" }; //TODO
    PixelSplitter_Bold_font = tgui::Font{ "fonts/PixelSplitter-Bold.ttf" };//TODO
	m_gui.setFont(PixelSplitter_Bold_font);//TODO
	tgui::Texture::setDefaultSmooth(false); // отключим сглаживание текстур //TODO

    m_ui = tgui::Group::create(); //TODO
	
	m_centered_message = tgui::Label::create("");
	m_centered_message->setPosition("50%", "50%");
	m_centered_message->setOrigin(0.5, 0.5);
	m_centered_message->setTextSize(128);
	m_centered_message->ignoreMouseEvents(true);
	m_centered_message->getRenderer()->setTextColor(tgui::Color::Red);

	m_ui->add(m_centered_message);

	// Нижняя панель

	auto bottom_panel_group = tgui::Group::create(tgui::Layout2d("100%", "10%"));
	bottom_panel_group->setPosition("0%", "90%");
	bottom_panel_group->setOrigin(0, 1.);

    auto help_button = tgui::BitmapButton::create("?");
    help_button->setImage(TextureManager::Instance().textures[TextureID::Question]);
    help_button->setImageScaling(1.);
    help_button->getRenderer()->setTexture(TextureManager::Instance().textures[TextureID::UpgradeButtonBackground]);
    help_button->getRenderer()->setBorders(0);
    help_button->onClick.connect([&]() {
        display_help(true);
    });
    help_button->setOrigin(1., 0.);
    help_button->setPosition("100%", 0);
    help_button->setVisible(true);
    bottom_panel_group->add(help_button, "HelpButton");
	bottom_panel_group->onSizeChange([=]() {
		const float spacing = 4.f;
		float size = bottom_panel_group->getSize().y;
        help_button->setSize({ size, size });
	});
    m_ui->add(bottom_panel_group, "BottomPanelGroup");

    m_gui.add(m_ui);

    m_help.get_content()->getRenderer()->setFont(GOSTtypeA_font);
    m_help.get_content()->setTextSize(30);

    m_mouse_tooltip = tgui::RichTextLabel::create();
    m_mouse_tooltip->setVisible(false);
    m_mouse_tooltip->setTextSize(30);
    m_mouse_tooltip->setOrigin(0, 1);
    m_mouse_tooltip->ignoreMouseEvents(true);
    auto mouse_tooltip_renderer = m_mouse_tooltip->getRenderer();
    mouse_tooltip_renderer->setFont(GOSTtypeA_font);
    mouse_tooltip_renderer->setBackgroundColor(sf::Color(50, 50, 50, 255));
    mouse_tooltip_renderer->setTextColor(sf::Color::White);

    m_ui->add(m_mouse_tooltip);

  
    TileMap::Instance().generate_map();
    m_console->add_message("Нажмите R чтобы перегенерировать карту и Q, чтобы подтвердить выбор.");
}

GameState::~GameState() {

}

tgui::Gui& GameState::get_tgui() {
	return m_gui;
}

void GameState::set_tooltip_content(const std::string& content, sf::Vector2f origin) {
    if (!content.empty()) {
        m_mouse_tooltip->setOrigin(origin.x, origin.y);
        m_mouse_tooltip->setText(content);
        m_mouse_tooltip->setVisible(true);
    }
    else {
        m_mouse_tooltip->setVisible(false);
    }
}

void GameState::set_wave_info(const std::string& wave) {
    m_wave_info->clear();
    m_wave_info->add_text(wave);
}

void GameState::add_enter(RoadGraph::PathID id, const std::string& content) {
    auto& path = EnemyManager::Instance().all_paths[id.start_node][id.path];
    Enter enter{ path[0]->x, path[0]->y, id, content, RouteDrawer(path) };
    m_enters.push_back(enter);
}

void GameState::delete_all_enters() {
    m_enters.clear();
    m_showed_enter = nullptr;
}

bool GameState::event(sf::Event& event, const sf::RenderWindow& current_window) {
    if (event.type == sf::Event::KeyPressed) {
        if (m_is_preparing) {
            if (event.key.code == sf::Keyboard::Key::R) {
                TileMap::Instance().generate_map();
            }
            else if (event.key.code == sf::Keyboard::Key::Q) {
                m_is_preparing = false;
                EnemyManager::Instance().generate_waves();
                init_stage(1);
            }
        }
    }
	if (event.type == sf::Event::MouseMoved) {
		sf::Vector2i mouse_screen_pos(event.mouseMove.x, event.mouseMove.y);
		m_mouse_pos = current_window.mapPixelToCoords(mouse_screen_pos);
        m_mouse_tooltip->setPosition(mouse_screen_pos.x, mouse_screen_pos.y);
        sf::Vector2i cell_id(m_mouse_pos.x / 32, m_mouse_pos.y / 32);
        bool mouse_on_enter = false;
        if (cell_id.x == 0 && m_mouse_pos.x < 0) {
            for (auto& enter : m_enters) {
                if (enter.y_id == cell_id.y) { // показать информацию о волне
                    if (m_showed_enter != &enter) {
                        set_tooltip_content(enter.content);
                        m_showed_enter = &enter;
                    }
                    mouse_on_enter = true;
                    break;
                }
            }
        }
        if (!mouse_on_enter && m_showed_enter) {
            set_tooltip_content("");
            m_showed_enter = nullptr;
        }
		return false;
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Left) {
            m_building_panel->build_if_allowed(m_mouse_pos);
		}
		else if (event.mouseButton.button == sf::Mouse::Button::Right) {
            m_building_panel->unselect();
            if (m_building_panel->is_seleted()) { // если мы строили, нужно отменить строительство
                m_building_panel->unselect();
            }
            else { // если не строили, значит запросили открыть окно постройки
                sf::Vector2i cell_id(m_mouse_pos.x / 32, m_mouse_pos.y / 32);
                size_t N = TileMap::Instance().map.size();
                bool on_map = cell_id.x < N && cell_id.x >= 0 && cell_id.y < N && cell_id.y >= 0;
                if (!on_map) {
                    m_game_process_ui->delete_widget(m_upgrade_panel);
                    m_upgrade_panel = nullptr;
                    return false;
                }
                auto& cell = TileMap::Instance().map[cell_id.x][cell_id.y];
                if (!cell.building) {
                    m_game_process_ui->delete_widget(m_upgrade_panel);
                    m_upgrade_panel = nullptr;
                    return false;
                }
                if (m_upgrade_panel) {
                    m_game_process_ui->delete_widget(m_upgrade_panel);
                    m_upgrade_panel = nullptr;
                }
                m_upgrade_panel = (UpgradePanel*)m_game_process_ui->add_widget(std::make_unique<UpgradePanel>(m_tile_size_reference, m_upgrade_panel_height_reference));
                m_upgrade_panel->position_anchor(Anchor::RIGHT | Anchor::TOP, m_player_health_count_widget, Anchor::RIGHT | Anchor::BOTTOM);
                cell.building->accept(*m_upgrade_panel);
            }
			return true;
		}
	}
	return false;
}

void GameState::logic(double dtime_mc) {
	if (is_player_defeated()) {
		m_centered_message->setText("GAME OVER");
		return;
	}
    if (m_win) {
        m_centered_message->setText("WIN, WIN, WIN!");
        return;
    }
    if (m_showed_enter) {
        m_showed_enter->drawer.logic(dtime_mc);
    }
    m_console->logic(dtime_mc);
}

void GameState::draw(sf::RenderWindow& current_window) {
    size_t N = TileMap::Instance().map.size();
	bool on_map = m_mouse_pos.x < N * 32 && m_mouse_pos.x >= 0 && m_mouse_pos.y < N * 32 && m_mouse_pos.y >= 0;
    sf::Vector2i cell_id(m_mouse_pos.x / 32, m_mouse_pos.y / 32);
    if (on_map)
        m_building_panel->draw_building_plan(current_window, cell_id.x, cell_id.y);
    if (!m_enters.empty()) {
        sf::Sprite wave_arrow(TextureManager::Instance().textures[TextureID::Arrow]);
        for (auto& enter : m_enters) {
            wave_arrow.setPosition((enter.x_id - 1) * 32, enter.y_id * 32);
            current_window.draw(wave_arrow);
        }
    }
    if (m_showed_enter) {
        m_showed_enter->drawer.draw(current_window);
    }
}

void GameState::enemy_defeated(EnemyType type) {
    bool achievement = AchievementSystem::Instance().defeated(type);
    if (!achievement)
        return;
    m_building_panel->update(m_player_coins);
    if (m_upgrade_panel)
        m_upgrade_panel->update(m_player_coins);
}

void GameState::win() {
    m_win = true;
}

void GameState::display_help(bool help) {
    m_is_help_displayed = help;
    m_gui.removeAllWidgets();
    if (help)
        m_gui.add(m_help.get_content());
    else
        m_gui.add(m_ui);
}

void GameState::init_stage(int stage) {
    if (stage == 0) {
        player_coins_add(1000);
        m_building_panel->update(m_player_coins);
    }
    else {
        AchievementSystem::Instance().unlock_all();
        player_coins_add(10000);
    }
}

void GameState::player_health_add(int health) {
	m_player_hp += health;
    m_player_health_count_widget->clear();
	m_player_health_count_widget->add_text("X" + std::to_string(m_player_hp));
}

void GameState::kill_player() {
    m_player_hp = 0;
    m_player_health_count_widget->clear();
    m_player_health_count_widget->add_text("X" + std::to_string(m_player_hp));
}

void GameState::player_coins_add(int coins) {
	m_player_coins += coins;
    m_player_coins_count_widget->clear();
    m_player_coins_count_widget->add_text(std::to_string(m_player_coins), Label::coins_color);
    m_building_panel->update(m_player_coins);
    if (m_upgrade_panel)
        m_upgrade_panel->update(m_player_coins);
}


void GameState::wave_preparing() {
    m_next_wave_button->set_active(true);
}

void GameState::wave_started() {
    m_next_wave_button->set_active(false);
}

void GameState::close_upgrade_panel() {
    m_game_process_ui->delete_widget(m_upgrade_panel);
    m_upgrade_panel = nullptr;
}

