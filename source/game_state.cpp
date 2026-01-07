#include "game_state.h"

#include "guns/mine.h"
#include "guns/minigun.h"
#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "achievement_system.h"
#include "enemy_manager.h"

GameState::GameState(sf::RenderWindow& window) : m_gui(window), window{window} {
    GOSTtypeA_font = tgui::Font{ "fonts/GOSTtypeA.ttf" };
    PixelSplitter_Bold_font = tgui::Font{ "fonts/PixelSplitter-Bold.ttf" };
	m_gui.setFont(PixelSplitter_Bold_font);
	tgui::Texture::setDefaultSmooth(false); // отключим сглаживание текстур

    m_ui = tgui::Group::create();

	m_player_health_count_widget = tgui::Label::create("X" + std::to_string(m_player_hp));
	m_player_health_count_widget->setPosition("100%", 0);
	m_player_health_count_widget->setOrigin(1, 0);
	m_player_health_count_widget->setTextSize(32);
	m_player_health_count_widget->ignoreMouseEvents(true);
	m_player_health_count_widget->getRenderer()->setTextColor(tgui::Color::White);
    m_ui->add(m_player_health_count_widget, "HealthCountWidget");


	tgui::Picture::Ptr heart_icon = tgui::Picture::create("sprites/heart.png");
	heart_icon->setPosition(
		"(HealthCountWidget.left - width)", // X: левый край rightWidget минус ширина heart_icon
		"(HealthCountWidget.top + 5)"       // Y: на той же высоте
	);
	heart_icon->setSize(32, 32);
	heart_icon->ignoreMouseEvents(true);
    m_ui->add(heart_icon);

	m_player_coins_count_widget = tgui::Label::create(std::to_string(m_player_coins));
	m_player_coins_count_widget->setTextSize(32);
	m_player_coins_count_widget->ignoreMouseEvents(true);
	m_player_coins_count_widget->getRenderer()->setTextColor(tgui::Color(255, 211, 3));

    m_wave_info = tgui::Label::create();
    m_wave_info->setTextSize(32);
    m_wave_info->ignoreMouseEvents(true);
    m_wave_info->setOrigin(0.5, 0);
    m_wave_info->setPosition("50%", 0);
    m_wave_info->getRenderer()->setTextColor(sf::Color::White);
    m_ui->add(m_player_coins_count_widget, "CoinsCountWidget");
    m_ui->add(m_wave_info);
	tgui::Picture::Ptr coin_picture = tgui::Picture::create("sprites/coin.png");
	coin_picture->setPosition(
		"(CoinsCountWidget.right)", // X: левый край rightWidget минус ширина heart_icon
		"(CoinsCountWidget.top + 5)"       // Y: на той же высоте
	);
	coin_picture->ignoreMouseEvents(true);
	coin_picture->setSize(32, 32);
    m_ui->add(coin_picture);

	m_centered_message = tgui::Label::create("");
	m_centered_message->setPosition("50%", "50%");
	m_centered_message->setOrigin(0.5, 0.5);
	m_centered_message->setTextSize(128);
	m_centered_message->ignoreMouseEvents(true);
	m_centered_message->getRenderer()->setTextColor(tgui::Color::Red);

	m_ui->add(m_centered_message);

	// Нижняя панель

	auto bottom_panel_group = tgui::Group::create(tgui::Layout2d("100%", "10%"));
	bottom_panel_group->setPosition("0%", "100%");
	bottom_panel_group->setOrigin(0, 1.);
	m_building_buttons.push_back(std::make_unique<MineBuildingButton>(*this));
	m_building_buttons.push_back(std::make_unique<MinigunBuildingButton>(*this));
	m_building_buttons.push_back(std::make_unique<AntitankGunBuildingButton>(*this));
	m_building_buttons.push_back(std::make_unique<TwinGunBuildingButton>(*this));
	m_building_buttons.push_back(std::make_unique<SpikesBuildingButton>(*this));
    m_building_buttons.push_back(std::make_unique<HedgeBuildingButton>(*this));
    m_building_buttons.push_back(std::make_unique<RadarBuildingButton>(*this));
	for (auto& button : m_building_buttons) {
		bottom_panel_group->add(button->m_group);
	}

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
    bottom_panel_group->add(help_button, "HelpButton");

    auto next_wave_button = tgui::BitmapButton::create();
    next_wave_button->setImage(TextureManager::Instance().textures[TextureID::NextWaveIcon]);
    next_wave_button->getRenderer()->setTexture(TextureManager::Instance().textures[TextureID::UpgradeButtonBackground]);
    next_wave_button->setOrigin(1., 0);
    next_wave_button->setPosition("HelpButton.left", 0);
    next_wave_button->setImageScaling(1.);
    next_wave_button->getRenderer()->setBorders(0);
    next_wave_button->onMousePress.connect([=]() {
        EnemyManager::Instance().start_wave();
        next_wave_button->getRenderer()->setTexture(TextureManager::Instance().textures[TextureID::UpgradeButtonBackgroundCompleted]);
    });
    next_wave_button->onMouseRelease.connect([=]() {
        next_wave_button->getRenderer()->setTexture(TextureManager::Instance().textures[TextureID::UpgradeButtonBackground]);
    });
    next_wave_button->onMouseEnter.connect([=]() {
        set_tooltip_content("Начать волну", { 1,0 });
    });
    next_wave_button->onMouseLeave.connect([=]() {
        set_tooltip_content("");
    });
    next_wave_button->setOrigin(1., 0.);

    bottom_panel_group->add(next_wave_button);

	bottom_panel_group->onSizeChange([=]() {
		const float spacing = 4.f;
		float size = bottom_panel_group->getSize().y;
		float x = 0;
		for (auto& button : m_building_buttons){
			button->m_group->setSize({ size, size });
			button->m_group->setPosition({ x, 0 });
			x += size + spacing;
		}
        help_button->setSize({ size, size });
        next_wave_button->setSize({ size, size });
	});

 

    m_ui->add(bottom_panel_group, "BottomPanelGroup");


    m_panel = tgui::Panel::create();
    m_panel->setTextSize(30);
    m_panel->setVisible(false);
    auto tooltip_renderer = m_panel->getRenderer();
    tooltip_renderer->setBackgroundColor(tgui::Color::Color(50, 50, 50, 200));
    tooltip_renderer->setBorders(3);
    tooltip_renderer->setBorderColor(tgui::Color::Black);
    tooltip_renderer->setFont(GOSTtypeA_font);
    m_panel->setPosition("100%", "HealthCountWidget.height");
    m_panel->setOrigin(1., 0);
    m_panel->setSize("25%", "80%");

    m_ui->add(m_panel);
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

    m_console = tgui::Group::create();
    m_console->setOrigin(0, 1.);
    m_console->setPosition(0, "BottomPanelGroup.top");
    m_console->setSize("100%", "90%");
    m_console->setTextSize(30);
    auto console_renderer = m_console->getRenderer();
    console_renderer->setFont(GOSTtypeA_font);
    m_console->onSizeChange.connect([this]() {
        this->align_console_labels();
    });
    m_ui->add(m_console);

    TileMap::Instance().generate_map();
    add_message("Нажмите R чтобы перегенерировать карту и Q, чтобы подтвердить выбор.", MessageType::None);
}

GameState::~GameState() {
    m_console->onSizeChange.disconnectAll(); // чтобы небыло Debug Assertion Failed
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

void GameState::add_enter(RoadGraph::PathID id, const std::string& content) {
    auto& path = EnemyManager::Instance().all_paths[id.start_node][id.path];
    Enter enter{ path[0]->x, path[0]->y, id, content, RouteDrawer(path) };
    m_enters.push_back(enter);
}

void GameState::delete_all_enters() {
    m_enters.clear();
    m_showed_enter = nullptr;
}

void GameState::add_message(const std::string& message, MessageType type) {
    auto label = tgui::Label::create(message);
    label->setTextSize(30);
    switch (type) {
    case MessageType::UnlockedBuilding:
        label->getRenderer()->setTextColor(sf::Color(0, 255, 0));
        break;
    case MessageType::UnlockedUpgrade:
        label->getRenderer()->setTextColor(sf::Color::Magenta);
        break;
    case MessageType::None:
        label->getRenderer()->setTextColor(sf::Color::White);
        break;
    default:
        break;
    }
    m_console->add(label);
    Message msg;
    msg.message = label;
    msg.animation.set_duration(4);
    auto& fade = msg.animation.add_subanimation(3, 4, Animation());
    fade.on_progress = [=](float p) {
        label->getRenderer()->setOpacity(1. - p);
    };
    msg.animation.start();
    m_messages.push_back(std::move(msg));
    align_console_labels();
}

void GameState::align_console_labels() {
    float y = 0;
    for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
        y += it->message->getSize().y;
        it->message->setPosition(0, m_console->getSize().y - y);
    }
}

bool GameState::event(sf::Event& event, const sf::RenderWindow& current_window) {
    if (event.type == sf::Event::KeyPressed) {
        if (m_is_preparing) {
            if (event.key.code == sf::Keyboard::Key::R) {
                TileMap::Instance().generate_map();
                m_prepairing_timer = 0;
                m_last_preparing_message = -1;
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
			if (m_current_building_construction) {
                size_t N = TileMap::Instance().map.size();
				bool on_map = m_mouse_pos.x < N * 32 && m_mouse_pos.x >= 0 && m_mouse_pos.y < N * 32 && m_mouse_pos.y >= 0;
				sf::Vector2i cell_id(m_mouse_pos.x / 32, m_mouse_pos.y / 32);
				if (on_map && m_current_building_construction->is_cell_allowed(cell_id.x, cell_id.y)) {
					TileMap::Instance().map[cell_id.x][cell_id.y].building = m_current_building_construction->creator();
					player_coins_add(-m_current_building_construction->cost);
					if (m_current_building_construction->get_state() == BuildingButton::State::Disabled) {
						m_current_building_construction = nullptr;
					}
					return true;
				}
			}
		}
		else if (event.mouseButton.button == sf::Mouse::Button::Right) {
            if (m_current_building_construction) { // если мы строили, нужно отменить строительство
                m_current_building_construction = nullptr;
                for (auto& btn : m_building_buttons)
                    btn->unselect();
            }
            else { // если не строили, значит запросили открыть окно постройки
                sf::Vector2i cell_id(m_mouse_pos.x / 32, m_mouse_pos.y / 32);
                size_t N = TileMap::Instance().map.size();
                bool on_map = cell_id.x < N && cell_id.x >= 0 && cell_id.y < N && cell_id.y >= 0;
                if (!on_map) {
                    set_panel_content(nullptr);
                    return false;
                }
                auto& cell = TileMap::Instance().map[cell_id.x][cell_id.y];
                if (!cell.building) {
                    set_panel_content(nullptr);
                    return false;
                }
                cell.building->accept(m_upgrade_panel_creator);
                set_panel_content(m_upgrade_panel_creator.panel);
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
    if (!m_messages.empty()) {
        bool erased = false;
        auto it = m_messages.begin();
        while (it != m_messages.end()) { // возможно можно переписать в помощью std::remove_if
            it->animation.logic(dtime_mc);
            if (it->animation.started())
                ++it;
            else {
                it = m_messages.erase(it);
                erased = true;
            }
        }
        if (erased) {
            m_console->removeAllWidgets();
            for (auto& msg : m_messages) {
                m_console->add(msg.message);
            }
            align_console_labels();
        }
    }
   /* if (m_is_preparing) {
        m_prepairing_timer += dtime_mc;
        int p = m_prepairing_timer / (1000.f * 1000.f);
        if (p > m_last_preparing_message) {
            m_last_preparing_message = p;
            add_message(std::to_string(4 - m_last_preparing_message) + " секунд до старта.", MessageType::None);
        }
        if (p == 4) {
            m_is_preparing = false;
            EnemyManager::Instance().generate_waves();
            init_stage(0);
        }
    }*/
}

void GameState::draw(sf::RenderWindow& current_window) {
    size_t N = TileMap::Instance().map.size();
	bool on_map = m_mouse_pos.x < N * 32 && m_mouse_pos.x >= 0 && m_mouse_pos.y < N * 32 && m_mouse_pos.y >= 0;
	if (m_current_building_construction && on_map) {
		sf::Vector2i cell_id(m_mouse_pos.x / 32, m_mouse_pos.y / 32);
		m_current_building_construction->draw_building_plan(current_window, cell_id.x, cell_id.y);
	}
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
    for (auto& btn : m_building_buttons)
        btn->defeat_event();
    m_upgrade_panel_creator.update();
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
    }
    else {
        player_coins_add(10000);
        AchievementSystem::Instance().unlock_all();
        enemy_defeated(EnemyType::CruiserI);
    }
}

void GameState::update_upgrade_panel() {
    m_upgrade_panel_creator.update();
}

void GameState::set_panel_content(tgui::Widget::Ptr content) {
    m_panel->removeAllWidgets();
    //m_upgrade_panel_creator.reset();
    if (content) {
        m_panel->add(content);
        m_panel->setVisible(true);
    }
    else {
        m_panel->setVisible(false);
    }
}

void GameState::player_health_add(int health) {
	m_player_hp += health;
	m_player_health_count_widget->setText("X" + std::to_string(m_player_hp));
}

void GameState::kill_player() {
    m_player_hp = 0;
    m_player_health_count_widget->setText("X" + std::to_string(m_player_hp));
}

void GameState::player_coins_add(int coins) {
	m_player_coins += coins;
	m_player_coins_count_widget->setText(std::to_string(m_player_coins));
	for (auto& btn : m_building_buttons)
		btn->coins_update(m_player_coins);
}

