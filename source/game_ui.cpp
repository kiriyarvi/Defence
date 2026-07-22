#include "game_ui.h"
#include "gui/scale.h"

#include "resource_manager.h"
#include "game_state.h"

#include "enemy_manager.h" 
#include <iostream>

GameUI::GameUI(GameState& game_state, sf::RenderWindow& window) :
    m_game_state{ game_state }, m_render_window{ window }, m_camera{window} {
}

void GameUI::create() {
    GUI::Instance().set_root(Widget::create(), m_render_window);
    Widget* root = GUI::Instance().get_root();
    DEBUG_TAG(root, "root");
    m_game_process_ui = root->add_widget(HoverableClickableWidget::create(ClickableWidget::Button::BOTH, ClickableWidget::Button::BOTH));
    m_game_process_ui->size_inherited(root);
    m_game_process_ui->capture_mode = false;
    m_game_process_ui->capture_mouse_move_only_if_no_obstacles = true;
    DEBUG_TAG(m_game_process_ui, "m_game_process_ui")

    //coin indicator (Hierarchy)
    m_player_coins_count_widget = (Label*)m_game_process_ui->add_widget(Label::create(true, 36, &ResourceManager::Instance().PixelSplitter_Bold_font));
    DEBUG_TAG(m_player_coins_count_widget, "m_player_coins_count_widget");
    m_player_coins_count_widget->add_text(std::to_string(m_game_state.get_player_coins()), Label::coins_color);
    Icon* coin_icon = (Icon*)m_game_process_ui->add_widget(Icon::create(TextureID::Coin));
    DEBUG_TAG(coin_icon, "coin_icon");
        //coin indicator (Layout)
    coin_icon->add_rule(Property::SIZE, [coins_counter = m_player_coins_count_widget](Widget::Layout& layout) {
        layout.width = coins_counter->layout.height;
        layout.height = coins_counter->layout.height;
    }, { {m_player_coins_count_widget, Property::SIZE} });
    coin_icon->position_anchor(Anchor::LEFT, m_player_coins_count_widget, Anchor::RIGHT);
    //health indicator (Hierarchy)
    m_player_health_count_widget = (Label*)m_game_process_ui->add_widget(Label::create(true, 36, &ResourceManager::Instance().PixelSplitter_Bold_font));
    DEBUG_TAG(m_player_health_count_widget, "m_player_health_count_widget")
        m_player_health_count_widget->add_text("X" + std::to_string(m_game_state.get_player_health()));
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
    DEBUG_TAG(m_wave_info, "m_wave_info");
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
    speed_controller->set_on_pos_update_callback([gs = &m_game_state](size_t pos) {
        gs->set_time_multiplier(1 + pos);
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
    }, { { m_game_process_ui, Property::HEIGHT } });
    //m_upgrade_panel_height_reference (вспомогательный виджет для панели апгрейдов), вычисляет высоту панели апгрейдов
    m_upgrade_panel_height_reference = m_game_process_ui->add_widget(Widget::create());
    m_upgrade_panel_height_reference->add_rule(Property::SIZE, [hc = m_player_health_count_widget, bp = m_building_panel, ui = m_game_process_ui](Widget::Layout& layout) {
        layout.height = ui->layout.height - hc->layout.height - bp->layout.height;
    }, { { m_player_health_count_widget, Property::HEIGHT }, {m_building_panel, Property::HEIGHT}, {m_game_process_ui, Property::HEIGHT} });

    //Console
    m_console = m_game_process_ui->add_widget(std::make_unique<Console>());
    m_console->position_anchor(Anchor::BOTTOM | Anchor::LEFT, m_building_panel, Anchor::TOP | Anchor::LEFT);

    //Enters Widget
    m_enters_widget = std::make_unique<EntersWidget>(m_game_process_ui);

    m_game_process_ui->set_on_hovered([this]() {
        auto mouse_screen_pos = GUI::Instance().mouse_pos;
        auto mouse_pos = m_render_window.mapPixelToCoords(sf::Vector2i( mouse_screen_pos.x, mouse_screen_pos.y ), m_camera.get_view());
        m_enters_widget->on_mouse_moved_on_map(mouse_pos);
    });
    m_game_process_ui->set_on_mouse_moved([this]() {
        auto mouse_screen_pos = GUI::Instance().mouse_pos;
        auto mouse_pos = m_render_window.mapPixelToCoords(sf::Vector2i(mouse_screen_pos.x, mouse_screen_pos.y), m_camera.get_view());
        m_enters_widget->on_mouse_moved_on_map(mouse_pos);
        m_game_process_ui->on_mouse_moved_return = Query{ Query::PASS }; //иначе событие не уйдет камере!
    });
    m_game_process_ui->set_on_unhovered([this]() {
        m_enters_widget->on_mouse_leave_map();
    });
    m_game_process_ui->set_on_pressed([this](ClickableWidget::Button::Type button) {
        on_button_pressed(button);
    });
    m_game_process_ui->set_on_released([this](ClickableWidget::Button::Type button) {
        m_game_process_ui->on_released_return = Query{ Query::PASS };
    });
}

void GameUI::on_button_pressed(ClickableWidget::Button::Type type) {
    auto mouse_screen_pos = GUI::Instance().mouse_pos;
    auto mouse_pos = m_render_window.mapPixelToCoords(sf::Vector2i(mouse_screen_pos.x, mouse_screen_pos.y), m_camera.get_view());
    if (type & ClickableWidget::Button::LEFT) {
        BuildingPanel::BuildResult res = m_building_panel->build_if_allowed({ mouse_pos.x, mouse_pos.y });
        if (res == BuildingPanel::BuildResult::NO_SELECTED_BUILDING_BUTTON)
            m_game_process_ui->on_pressed_return = Query{ Query::PASS }; //событие не обрабатываем.
    } else if (type & ClickableWidget::Button::RIGHT) {
        m_building_panel->unselect();
        if (m_building_panel->is_seleted()) { // если мы строили, нужно отменить строительство
            m_building_panel->unselect();
        }
        else { // если не строили, значит запросили открыть окно постройки
            GUI::Instance().add_deffered_command([mouse_pos, this]() {
                sf::Vector2i cell_id(mouse_pos.x / 32, mouse_pos.y / 32);
                size_t N = m_game_state.get_map().map.size();
                bool on_map = cell_id.x < N && cell_id.x >= 0 && cell_id.y < N && cell_id.y >= 0;
                if (!on_map) {
                    m_game_process_ui->delete_widget(m_upgrade_panel);
                    m_upgrade_panel = nullptr;
                    return;
                }
                auto& cell = m_game_state.get_map().map[cell_id.x][cell_id.y];
                if (!cell.building) {
                    m_game_process_ui->delete_widget(m_upgrade_panel);
                    m_upgrade_panel = nullptr;
                    return;
                }
                if (m_upgrade_panel) {
                    m_game_process_ui->delete_widget(m_upgrade_panel);
                    m_upgrade_panel = nullptr;
                }
                if (!m_upgrade_panel->is_interface_available(cell.building->type))
                    return;
                m_upgrade_panel = (UpgradePanel*)m_game_process_ui->add_widget(std::make_unique<UpgradePanel>(m_tile_size_reference, m_upgrade_panel_height_reference));
                m_upgrade_panel->position_anchor(Anchor::RIGHT | Anchor::TOP, m_player_health_count_widget, Anchor::RIGHT | Anchor::BOTTOM);
                cell.building->accept(*m_upgrade_panel);
            });
        }
    }
}

void GameUI::on_event(sf::Event& event) {
    if (!GUI::Instance().event(event))
        m_camera.process(event);
    if (event.type == sf::Event::KeyPressed) {
        if (m_game_state.get_state() == GameState::State::PREPAIRING) {
            if (event.key.code == sf::Keyboard::Key::R)
                m_game_state.get_map().generate_map();
            else if (event.key.code == sf::Keyboard::Key::Q) {
                m_game_state.start_game();
            }
        }
        //DEBUG
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::E) {
            m_game_state.get_map().enlarge_map();
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::K) {
            for (auto& enemy : EnemyManager::Instance().m_enemies)
                enemy->health = 0;
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::T) {
            m_game_state.get_map().create_tile_test_map();
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
            sf::Vector2i mouse_screen_pos = sf::Mouse().getPosition();
            auto mouse_pos = m_render_window.mapPixelToCoords(mouse_screen_pos);
            EnemyManager::Instance().add_smoke(Smoke({ mouse_pos.x, mouse_pos.y }, 4., 20.));
        }
    }
}

void GameUI::logic(float dtime_mc) {
    m_enters_widget->logic(dtime_mc);
    m_console->logic(dtime_mc);
}

void GameUI::update_player_health(int health) {
    m_player_health_count_widget->clear();
    m_player_health_count_widget->add_text("X" + std::to_string(health));
}

void GameUI::update_player_coins(int coins) {
    m_player_coins_count_widget->clear();
    m_player_coins_count_widget->add_text(std::to_string(coins), Label::coins_color);
    m_building_panel->update(coins);
    if (m_upgrade_panel)
        m_upgrade_panel->update(coins);
}

void GameUI::update_wave_info(bool wave_started) {
    m_next_wave_button->set_active(!wave_started);
}

void GameUI::update_on_enemy_defeated(int coins) {
    m_building_panel->update(coins);
    if (m_upgrade_panel)
        m_upgrade_panel->update(coins);
}

void GameUI::game_over(bool win) {
    Widget* root = GUI::Instance().get_root();
    Label* label = root->add_widget(Label::create(true, 128, &ResourceManager::Instance().PixelSplitter_Bold_font));
    if (win)
        label->add_text("WIN!", sf::Color::Red);
    else
        label->add_text("DEFEAT!", sf::Color::Red);
    label->position_centering();
}

void GameUI::close_upgrade_panel() {
    m_game_process_ui->delete_widget(m_upgrade_panel);
    m_upgrade_panel = nullptr;
}


void GameUI::update_wave_indicator_text(const std::string& text) {
    m_wave_info->clear();
    m_wave_info->add_text(text);
}

void GameUI::draw_on_map_effects() {
    auto mouse_screen_pos = GUI::Instance().mouse_pos;
    auto mouse_pos = m_render_window.mapPixelToCoords(sf::Vector2i(mouse_screen_pos.x, mouse_screen_pos.y), m_camera.get_view());
    size_t N = m_game_state.get_map().map.size();
    bool on_map = mouse_pos.x < N * 32 && mouse_pos.x >= 0 && mouse_pos.y < N * 32 && mouse_pos.y >= 0;
    sf::Vector2i cell_id(mouse_pos.x / 32, mouse_pos.y / 32);
    if (on_map)
        m_building_panel->draw_building_plan(m_render_window, cell_id.x, cell_id.y);
    m_enters_widget->draw(m_render_window);
}
