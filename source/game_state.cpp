#include "game_state.h"

#include "guns/mine.h"
#include "guns/minigun.h"
#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "achievement_system.h"

GameState::GameState(sf::RenderWindow& window): m_gui(window) {
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
    m_ui->add(m_player_coins_count_widget, "CoinsCountWidget");
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
	for (auto& button : m_building_buttons) {
		bottom_panel_group->add(button->group);
	}

    auto help_button = tgui::Button::create("?");
    help_button->onClick.connect([&]() {
        display_help(true);
    });
    help_button->setOrigin(1., 0.);
    help_button->setPosition("100%", 0);
    bottom_panel_group->add(help_button);

	bottom_panel_group->onSizeChange([=]() {
		const float spacing = 4.f;
		float size = bottom_panel_group->getSize().y;
		float x = 0;
		for (auto& button : m_building_buttons){
			button->group->setSize({ size, size });
			button->group->setPosition({ x, 0 });
			x += size + spacing;
		}
        help_button->setSize({ size, size });
	});

 

    m_ui->add(bottom_panel_group);

	player_coins_add(2000);

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
}

tgui::Gui& GameState::get_tgui() {
	return m_gui;
}

void GameState::set_tooltip_content(const std::string& content) {
    if (!content.empty()) {
        m_mouse_tooltip->setText(content);
        m_mouse_tooltip->setVisible(true);
    }
    else {
        m_mouse_tooltip->setVisible(false);
    }
}

bool GameState::event(sf::Event& event, const sf::RenderWindow& current_window) {
	if (event.type == sf::Event::MouseMoved) {
		sf::Vector2i mouse_screen_pos(event.mouseMove.x, event.mouseMove.y);
		m_mouse_pos = current_window.mapPixelToCoords(mouse_screen_pos);
        m_mouse_tooltip->setPosition(mouse_screen_pos.x, mouse_screen_pos.y);
		return false;
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Left) {
			if (m_current_building_construction) {
				bool on_map = m_mouse_pos.x < 8 * 32 && m_mouse_pos.x >= 0 && m_mouse_pos.y < 8 * 32 && m_mouse_pos.y >= 0;
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
			m_current_building_construction = nullptr;
			for (auto& btn : m_building_buttons)
				btn->unselect();
			return true;
		}
	}
	return false;
}

void GameState::logic() {
	if (is_player_defeated()) {
		m_centered_message->setText("GAME OVER");
		return;
	}
    if (m_win) {
        m_centered_message->setText("WIN, WIN, WIN!");
        return;
    }
}

void GameState::draw(sf::RenderWindow& current_window) {
	bool on_map = m_mouse_pos.x < 8 * 32 && m_mouse_pos.x >= 0 && m_mouse_pos.y < 8 * 32 && m_mouse_pos.y >= 0;
	if (m_current_building_construction && on_map) {
		sf::Vector2i cell_id(m_mouse_pos.x / 32, m_mouse_pos.y / 32);
		m_current_building_construction->draw_building_plan(current_window, cell_id.x, cell_id.y);
	}
}

void GameState::enemy_defeated(EnemyType type) {
    AchievementSystem::Instance().defeated(type);
    for (auto& btn : m_building_buttons)
        btn->defeat_event();
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

void GameState::set_panel_content(tgui::Widget::Ptr content) {
    m_panel->removeAllWidgets();
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

