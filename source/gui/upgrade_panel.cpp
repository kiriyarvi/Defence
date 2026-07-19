#include "gui/upgrade_panel.h"
#include "gui/label.h"
#include "achievement_system.h"
#include "game_state.h"

UpgradeButton::UpgradeButton(
    UpgradePanel* upgrade_panel,
    UpgradeButton* prev_upgrade_button,
    IBuilding* building,
    Upgrade* upgrade,
    int level,
    TextureID upgrade_icon):
    m_upgrade_panel{upgrade_panel},
    m_prev_upgrade_button{prev_upgrade_button},
    m_building{building},
    m_upgrade{upgrade},
    m_level{level},
    m_upgrade_icon{upgrade_icon}
{
    set_state(m_state);
}

void UpgradeButton::update(int player_coins) {
    if (m_state == State::BOUGTH)
        return;
    if (m_state == State::UNDISCOVERED) {
        if ((m_prev_upgrade_button && m_prev_upgrade_button->get_state() == State::UNDISCOVERED) || m_upgrade->available_level < m_level)
            return; //остается UNDISCOVERED
    }
    State state = player_coins >= m_upgrade->cost(m_level) ? State::ACTIVE : State::NOT_ENOGTH_MONEY;
    set_state(state);
}

Query UpgradeButton::on_event(EventContext event_context) {
    if (event_context.event_type == Event::BUTTON_PRESSED) {
        if (GUI::Instance().mouse_button == sf::Mouse::Left && m_state != State::ACTIVE)
            return Query{ Query::PROCESSED }; //взаимодействия точно не будет
        //потенциальное взаимодействие => подпишемся
        GUI::Instance().subscribe_deffered(this, Event::BUTTON_RELEASED | Event::MOUSE_MOVED);
        return Query{ Query::PROCESSED };
    }
    else if (event_context.event_type == Event::BUTTON_RELEASED) {
        if (GUI::Instance().mouse_button == sf::Mouse::Left) { //попытка купить
            if (m_state == State::ACTIVE) { //нужно проверить, пока мышь была зажата, деньги могли уменьшиться (автовостановление шипов)
                m_upgrade->upgrade(m_building, m_level); //апгрейдим до нужного уровня.
                set_state(State::BOUGTH);
            }
        }
        else { //правая кнопка => выделим
            m_upgrade_panel->capture(this);
        }
        GUI::Instance().unsubscribe_deffered(this, Event::BUTTON_RELEASED | Event::MOUSE_MOVED);
        return Query{ Query::PROCESSED };
    }
    else
        return Query::skip(event_context.from_subscribe);
}



void UpgradeButton::set_state(State state) {
    m_state = state;
    switch (state) {
    case UpgradeButton::State::ACTIVE:
        layers = { TextureID::UpgradeButtonBackground, m_upgrade_icon };
        grayscale = false;
        break;
    case UpgradeButton::State::BOUGTH:
        layers = { TextureID::UpgradeButtonBackgroundCompleted, m_upgrade_icon };
        grayscale = false;
        break;
    case UpgradeButton::State::NOT_ENOGTH_MONEY:
        layers = { TextureID::UpgradeButtonBackground, m_upgrade_icon };
        grayscale = true;
        break;
    case UpgradeButton::State::UNDISCOVERED:
        layers = { TextureID::UpgradeButtonBackground, m_upgrade_icon, TextureID::Locked };
        grayscale = true;
        break;
    default:
        break;
    }
}

void UpgradeButton::set_capture(bool capture) {
    if (m_captured != capture) {
        m_captured = capture;
        if (m_captured) {
            auto capture_icon = Icon::create(TextureID::ButtonCapture);
            m_capture_icon = capture_icon.get();
            add_widget_deffered(std::move(capture_icon));
            m_capture_icon->size_inherited(this);
        }
        else {
            delete_widget_deffered(m_capture_icon, RemovePolicy::Min);
            m_capture_icon = nullptr;
        }
    }
}


UpgradePanel::UpgradePanel(Widget* tile_size_reference, Widget* height_reference):
    TiledPanel(TiledPanel::Type::Blueprint, tile_size_reference),
    m_tile_size_reference(tile_size_reference), m_height_reference(height_reference)
{
    clear_rules(Property::LAYOUT);

    add_rule(Property::SIZE, [this](Layout& layout) {
        m_cached_tile_size = m_tile_size_reference->layout.width;
        m_cached_tiles_count.x = std::max<float>((content_widget->layout.width + m_cached_tile_size) / m_cached_tile_size, 2);
        m_cached_tiles_count.y = std::max<float>(m_height_reference->layout.height / m_cached_tile_size, 2);
        layout.width = m_cached_tile_size * m_cached_tiles_count.x;
        layout.height = m_cached_tile_size * m_cached_tiles_count.y;
    }, { { content_widget, Property::WIDTH }, {m_tile_size_reference, Property::SIZE }, {m_height_reference, Property::HEIGHT} });

    content_widget->clear_rules(Property::LAYOUT);
    content_widget->add_rule(Property::POSITION, [upgrade_panel = this](Layout& layout) {
        layout.x = (upgrade_panel->layout.width - layout.width) / 2.f;
        layout.y = upgrade_panel->m_cached_tile_size * 0.25;
    }, { { this, Property::WIDTH }, {content_widget, Property::WIDTH} }); //центрироввание по x.

}

void UpgradePanel::capture(UpgradeButton* button) {
    for (UpgradeButton* up_button : m_upgrade_buttons) {
        if (up_button == button)
            up_button->set_capture(true);
        else
            up_button->set_capture(false);
    }
}

Widget* UpgradePanel::create_buttons_for_upgrade(Widget* parent, IBuilding* building, Upgrade* upgrade, const std::vector<TextureID>& upgrade_buttons_icons) {
    Widget* upgrades_panel = parent->add_widget(Widget::create());
    std::vector<Widget*> upgrade_buttons;
    for (size_t i = 1; i <= upgrade->max_level; ++i) {
        UpgradeButton* button = (UpgradeButton*)upgrades_panel->add_widget(std::make_unique<UpgradeButton>(
            this,
            static_cast<UpgradeButton*>(i == 1 ? nullptr : upgrade_buttons.back()),
            building,
            upgrade,
            i,
            upgrade_buttons_icons[i - 1]
        ));
        button->add_rule(Property::SIZE, [p = m_parent](Layout& layout) {
            layout.width = 0.1 * p->layout.height;
            layout.height = 0.1 * p->layout.height;
        }, { {m_parent, Property::HEIGHT } });
        static_cast<UpgradeButton*>(button)->update(GameState::Instance().get_player_coins());
        upgrade_buttons.push_back(button);
        m_upgrade_buttons.push_back(button);
    }
    upgrades_panel->hbox(upgrade_buttons);
    return upgrades_panel;
}

void UpgradePanel::visit(MiniGun& minigun) {
    auto& achievement_system = AchievementSystem::Instance();
    Widget* upgrade_buttons_panel = add_widget(Widget::create());
    Widget* penetration_upgrades_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &minigun, &achievement_system.minigun_penetration_upgrade, { TextureID::MinigunShellsUpgradeI, TextureID::MinigunShellsUpgradeII, TextureID::MinigunShellsUpgradeIII });
    Widget* cooling_upgrade_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &minigun, &achievement_system.minigun_cooling_upgrade, { TextureID::MinigunCoolingUpgradeI, TextureID::MinigunCoolingUpgradeI, TextureID::MinigunCoolingUpgradeI });
    Widget* lubricant_upgrade_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &minigun, &achievement_system.minigun_lubricant_upgrade, { TextureID::MinigunLubricantUpgradeI, TextureID::MinigunLubricantUpgradeI, TextureID::MinigunLubricantUpgradeI });
    upgrade_buttons_panel->vbox({ penetration_upgrades_panel, cooling_upgrade_panel, lubricant_upgrade_panel });

    content_widget->size_include(upgrade_buttons_panel);
    
}

void UpgradePanel::visit(Spikes& spikes) {

}

void UpgradePanel::visit(Hedgehog& headgehogs) {

}

void UpgradePanel::visit(AntitankGun& antitank_gun) {

}

void UpgradePanel::visit(TwinGun& twingun) {

}

void UpgradePanel::visit(Mine& mine) {

}

void UpgradePanel::visit(Radar& radar) {

}

void UpgradePanel::visit(RadioMast& radio_tower) {

}





//#include "tile_map.h"
//#include <vector>
//#include "guns/minigun.h"
//#include "guns/radar.h"
//#include "guns/radio_mast.h"
//#include "game_state.h"
//#include "achievement_system.h"
//#include "gui/info_panel.h"
//#include "net_manager.h"
//
//UpgradeButton::UpgradeButton(
//    TextureID icon,
//    int cost,
//    UpgradeableProperty& building_upgrade,
//    int& achievement_system_upgrade,
//    int goal_upgrade_value,
//    const std::string name
//):
//    m_cost(cost), m_building_upgrade(building_upgrade), m_achievement_system_upgrade(achievement_system_upgrade),
//    m_goal_upgrade_value(goal_upgrade_value), m_name(name),
//    IconButton(icon, TextureID::UpgradeButtonBackground, TextureID::UpgradeButtonBackgroundCompleted) {
//    connect();
//    update();
//}
//
//UpgradeButton::UpgradeButton(UpgradeButton&& btn): IconButton(std::move(btn)),
//    m_building_upgrade(btn.m_building_upgrade),
//    m_achievement_system_upgrade(btn.m_achievement_system_upgrade),
//    m_goal_upgrade_value(btn.m_goal_upgrade_value), m_name(btn.m_name), m_reason{btn.m_reason}
//{
//    m_cost = btn.m_cost;
//    on_mouse_enter = btn.on_mouse_enter;
//    on_mouse_leave = btn.on_mouse_leave;
//    connect();
//}
//
//void UpgradeButton::connect() {
//    m_button->onPress.disconnectAll();
//    m_button->onPress.connect([&]() {
//        if (m_state == State::Active) {
//            m_building_upgrade = m_goal_upgrade_value;
//            GameState::Instance().player_coins_add(-m_cost);
//            GameState::Instance().update_upgrade_panel();
//        }
//    });
//    m_button->onMouseEnter.disconnectAll();
//    m_button->onMouseEnter.connect([&]() {
//        GameState::Instance().set_tooltip_content(
//           "<b>"+  m_name + "</b>\n<color=#ffd303>Стоимость: " + std::to_string(m_cost) + "</color>" + m_reason
//            , {1.,0.});
//        if (on_mouse_enter) on_mouse_enter();
//    });
//    m_button->onMouseLeave.disconnectAll();
//    m_button->onMouseLeave.connect([&]() {
//        GameState::Instance().set_tooltip_content("");
//        if (on_mouse_leave) on_mouse_leave();
//    });
//}
//
//
//
//void UpgradeButton::update() {
//    m_reason = "";
//    if (m_building_upgrade >= m_goal_upgrade_value) {
//        set_state(State::Selected);
//    }
//    else { // еще не достигли нужного уровня
//        bool locked = false;
//        if (m_goal_upgrade_value > m_building_upgrade + 1) { // закрыто, так как нужно купить предыдущий уровень
//            locked = true;
//            m_reason = "\n<color=#ff0000>Закрыто: </color> требуется предыдущее улучшение.";
//        }
//        if (m_achievement_system_upgrade < m_goal_upgrade_value) {
//            m_reason += "\n<color=#ff0000>Закрыто: </color> " + AchievementSystem::Instance().get_upgrade_unlock_condition_description(&m_achievement_system_upgrade, m_goal_upgrade_value) + "."; 
//            locked = true;
//        }
//        if (locked) {
//            set_state(State::Locked);
//        }
//        else { // открыто
//            int current_coins_count = GameState::Instance().get_player_coins();
//            if (current_coins_count >= m_cost)
//                set_state(State::Active);
//            else
//                set_state(State::Disabled);
//        }
//    }
//}
//
//
//UpgradePanelCreator::UpgradePanelCreator() {
//    panel = tgui::Group::create();
//    panel->setSize("100%", "100%");
//}
//
//void UpgradePanelCreator::reset() {
//    m_buttons.clear();
//    panel->removeAllWidgets();
//    for (auto& action : m_clear_actions)
//        action();
//    m_clear_actions.clear();
//}
//
//void UpgradePanelCreator::visit(MiniGun& minigun) {
//    reset();
//
//    m_buttons.resize(3);
//    auto& shells_upgrades = m_buttons[0];
//    auto& cooling_upgrades = m_buttons[1];
//    auto& lubricant_upgrades = m_buttons[2];
//
//    auto& params = ParamsManager::Instance().params.guns.minigun;
//    UpgradeButton shells_upgrade_I(TextureID::MinigunShellsUpgradeI, params.penetration_upgrades[1].cost, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 1, "Бронебойные снаряды I");
//    UpgradeButton shells_upgrade_II(TextureID::MinigunShellsUpgradeII, params.penetration_upgrades[2].cost, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 2, "Бронебойные снаряды II");
//    UpgradeButton shells_upgrade_III(TextureID::MinigunShellsUpgradeIII, params.penetration_upgrades[3].cost, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 3, "Бронебойные снаряды III");
//    shells_upgrades.push_back(std::move(shells_upgrade_I));
//    shells_upgrades.push_back(std::move(shells_upgrade_II));
//    shells_upgrades.push_back(std::move(shells_upgrade_III));
//    for (size_t i = 0; i < 3;++i) {
//        shells_upgrades[i].on_mouse_enter = [&, i]() {
//            auto& up = shells_upgrades[i];
//            InfoPanel panel;
//            panel.set_name("Бронебойные снаряды " + std::string(i + 1, 'I'));
//            panel.set_description("Пулемет получит снаряды повышенной бронепробиваемости.");
//            panel.add_char("бронепробиваемость при минимальном нагреве", params.penetration_upgrades[i + 1].min_armor_penetration_level);
//            panel.add_char("бронепробиваемость при максимальном нагреве", params.penetration_upgrades[i + 1].max_armor_penetration_level);
//            panel.add_char("минимальный урон", params.penetration_upgrades[i + 1].min_damage);
//            panel.add_char("максимальный урон", params.penetration_upgrades[i + 1].max_damage);
//            panel.create();
//            info->removeAllWidgets();
//            info->add(panel.content);
//        };
//    }
//    UpgradeButton cooling_upgrade_I(TextureID::MinigunCoolingUpgradeI, params.cooling_upgrades[1].cost, minigun.m_cooling_upgrade, AchievementSystem::Instance().minigun_upgrades.cooling_upgrade, 1, "Система охлаждения I");
//    UpgradeButton cooling_upgrade_II(TextureID::MinigunCoolingUpgradeI, params.cooling_upgrades[2].cost, minigun.m_cooling_upgrade, AchievementSystem::Instance().minigun_upgrades.cooling_upgrade, 2, "Система охлаждения II");
//    UpgradeButton cooling_upgrade_III(TextureID::MinigunCoolingUpgradeI, params.cooling_upgrades[3].cost, minigun.m_cooling_upgrade, AchievementSystem::Instance().minigun_upgrades.cooling_upgrade, 3, "Система охлаждения III");
//    cooling_upgrades.push_back(std::move(cooling_upgrade_I));
//    cooling_upgrades.push_back(std::move(cooling_upgrade_II));
//    cooling_upgrades.push_back(std::move(cooling_upgrade_III));
//    for (size_t i = 0; i < 3; ++i) {
//        cooling_upgrades[i].on_mouse_enter = [&, i]() {
//            auto& up = cooling_upgrades[i];
//            InfoPanel panel;
//            panel.set_name("Система охлаждения " + std::string(i + 1, 'I'));
//            panel.set_description("Пулемет получит улучшенную систему охлаждения, продливающую время работы при критическом нагреве, а также увеличивающую скорость охлаждения.");
//            panel.add_char("время работы при критическом перегреве", params.cooling_upgrades[i + 1].critical_temperature_work_duration);
//            panel.add_char("время полного охлаждения", params.cooling_upgrades[i + 1].cooling_time);
//            panel.add_char("время на охлаждение после перегрева", params.cooling_upgrades[i + 1].cooldown_duration);
//            panel.create();
//            info->removeAllWidgets();
//            info->add(panel.content);
//        };
//    }
//    UpgradeButton lubricant_upgrade_I(TextureID::MinigunLubricantUpgradeI, params.lubricant_upgrades[1].cost, minigun.m_lubricant_upgrade, AchievementSystem::Instance().minigun_upgrades.lubricant_update, 1, "Смазка I");
//    UpgradeButton lubricant_upgrade_II(TextureID::MinigunLubricantUpgradeI, params.lubricant_upgrades[2].cost, minigun.m_lubricant_upgrade, AchievementSystem::Instance().minigun_upgrades.lubricant_update, 2, "Смазка II");
//    UpgradeButton lubricant_upgrade_III(TextureID::MinigunLubricantUpgradeI, params.lubricant_upgrades[3].cost, minigun.m_lubricant_upgrade, AchievementSystem::Instance().minigun_upgrades.lubricant_update, 3, "Смазка III");
//    lubricant_upgrades.push_back(std::move(lubricant_upgrade_I));
//    lubricant_upgrades.push_back(std::move(lubricant_upgrade_II));
//    lubricant_upgrades.push_back(std::move(lubricant_upgrade_III));
//    for (size_t i = 0; i < 3; ++i) {
//        lubricant_upgrades[i].on_mouse_enter = [&, i]() {
//            auto& up = lubricant_upgrades[i];
//            InfoPanel panel;
//            panel.set_name("Смазка " + std::string(i + 1, 'I'));
//            panel.set_description("Пулемет получит улучшенную смачную систему, что позволит ему набирать максимальную скорость вращения барабана быстрее. При этом соответсвие скорости и нагрева останется прежним.");
//            panel.add_char("Время нагрева до максимальной температуры", params.lubricant_upgrades[i + 1].heating_time);
//            panel.create();
//            info->removeAllWidgets();
//            info->add(panel.content);
//        };
//    }
//
//    tgui::Grid::Ptr grid = tgui::Grid::create();
//    
//    for (int cat = 0; cat < m_buttons.size(); ++cat)
//        for (int up = 0; up < m_buttons[cat].size(); ++up) {
//            grid->addWidget(m_buttons[cat][up].m_group, cat, up);
//            float size = GameState::Instance().window.getSize().y * 0.1;
//            m_buttons[cat][up].m_group->setSize(size, size);
//        }
//    panel->add(grid, "Grid");
//
//    m_compute_cost = [&]() {
//        int cost = params.cost;
//        for (int i = 1; i <= minigun.m_lubricant_upgrade; ++i) cost += params.lubricant_upgrades[i].cost;
//        for (int i = 1; i <= minigun.m_cooling_upgrade; ++i) cost += params.cooling_upgrades[i].cost;
//        for (int i = 1; i <= minigun.m_penetration_upgrade; ++i) cost += params.penetration_upgrades[i].cost;
//        return cost;
//    };
//    auto sell_button = create_sell_button(minigun.x_id, minigun.y_id);
//    sell_button->setPosition(0, "Grid.bottom");
//    panel->add(sell_button, "SellButton");
//
//    info = tgui::Group::create();
//    info->setPosition(0, "SellButton.bottom");
//    panel->add(info);
//}
//
//template<typename Params, typename Building>
//tgui::Group::Ptr create_panel_for_building_with_health(UpgradePanelCreator* creator, BuildingType type, Params& params, Building& building) {
//    auto label = tgui::RichTextLabel::create();
//    auto update = [&building, creator, label = label.get(), type]() {
//        if (building.get_health() <= 0) {
//            creator->reset();
//            return;
//        }
//        label->setText("<b>" + to_string(type) + "</b>\nПрочность: " + std::to_string(building.get_health()));
//    };
//    update();
//    label->getRenderer()->setTextColor(sf::Color::White);
//    building.set_health_changed_callback(update);
//    creator->m_clear_actions.push_back([&building]() {
//        building.set_health_changed_callback({});
//    });
//
//    auto description = tgui::Label::create();
//    description->setVisible(false);
//    description->getRenderer()->setTextColor(sf::Color::White);
//    auto button = tgui::Button::create("Укрепить");
//    button->onMouseEnter.connect([&params, description = description.get()]() {
//        description->setVisible(true);
//        description->setText("Увеличит прочность на " + std::to_string(params.health)  + " единиц.");
//        GameState::Instance().set_tooltip_content("<color=#ffd303>Стоимость: " + std::to_string(params.cost) + "</color>", { 1.,0. });
//    });
//    button->onMouseLeave.connect([description = description.get()]() {
//        description->setVisible(false);
//        GameState::Instance().set_tooltip_content("");
//    });
//    button->onClick.connect([&params, &building]() {
//        int coins = GameState::Instance().get_player_coins();
//        if (coins >= params.cost) {
//            building.set_health(building.get_health() + 5);
//            GameState::Instance().player_coins_add(-params.cost);
//        }
//    });
//
//    auto auto_repair_button = tgui::ToggleButton::create("Автовосстановление");
//    auto_repair_button->onMouseEnter.connect([type, &params, description = description.get()]() {
//        description->setVisible(true);
//        description->setText("Включает автовосстановление. При достижении нулевой прочности и наличии достаточных средств, " + to_string(type) + " автоматически восстаноят 5 единиц прочности.");
//    });
//    auto_repair_button->onMouseLeave.connect([description = description.get()]() {
//        description->setVisible(false);
//    });
//    auto_repair_button->onToggle([&building](bool isDown) {
//        building.auto_repair = isDown;
//    });
//    auto_repair_button->setDown(building.auto_repair);
//
//
//
//    auto group = tgui::Group::create();
//    group->setSize("100%", "100%");
//
//    group->add(label, "Header");
//    button->setPosition(0, "Header.bottom");
//    group->add(button, "Endurance");
//    auto_repair_button->setPosition(0, "Endurance.bottom");
//    group->add(auto_repair_button, "Repair");
//    description->setPosition(0, "Repair.bottom");
//    group->add(description);
//    group->onSizeChange.connect([group = group.get(), description = description.get()]() {
//        description->setMaximumTextWidth(group->getSize().x);
//    });
//    return group;
//}
//
//tgui::Button::Ptr UpgradePanelCreator::create_sell_button(int x_id, int y_id) {
//    auto sell_button = tgui::Button::create("продать");
//    sell_button->onClick([x_id, y_id, this]() {
//        GameState::Instance().player_coins_add(int(m_compute_cost() * 0.5));
//        TileMap::Instance().delete_building(x_id, y_id);
//        reset();
//    });
//    sell_button->onMouseEnter([this]() {
//        auto content = tgui::ScrollablePanel::create();
//        content->setSize("100%", "100%");
//        content->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Automatic);
//        content->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
//        content->getRenderer()->setBackgroundColor(sf::Color::Transparent);
//        content->setContentSize({ 10000, 3000 });
//
//        auto text = tgui::RichTextLabel::create("<color=#ffd303>" + std::to_string(int(m_compute_cost() * 0.5)) + "</color> (половина от стоимости с учетом улучшений)");
//        text->getRenderer()->setTextColor(tgui::Color::White);
//        text->onSizeChange.connect([d = text, c = content]() {
//            d->setMaximumTextWidth(c->getSize().x);
//        });
//        content->add(text);
//        info->removeAllWidgets();
//        info->add(content);
//    });
//    return sell_button;
//    
//}
//
//void UpgradePanelCreator::visit(Spikes& spikes) {
//    // TODO этот код почему-то вызывает ошибку при закрытии программы
//    reset();   
//    panel->add(create_panel_for_building_with_health(this, BuildingType::Spikes, ParamsManager::Instance().params.guns.spikes, spikes));
//}
//
//void UpgradePanelCreator::visit(Hedgehog& headgehogs) {
//    // TODO этот код почему-то вызывает ошибку при закрытии программы
//    reset();
//    panel->add(create_panel_for_building_with_health(this, BuildingType::Hedgehogs, ParamsManager::Instance().params.guns.hedgehog, headgehogs));
//}
//
//void UpgradePanelCreator::visit(AntitankGun& antitank_gun) {
//    reset();
//}
//
//void UpgradePanelCreator::visit(TwinGun& twingun) {
//    reset();
//}
//
//void UpgradePanelCreator::visit(Mine& mine) {
//    reset();
//}
//
//void UpgradePanelCreator::visit(Radar& radar) {
//    reset();
//
//    m_buttons.resize(4);
//    auto& radius_upgrades = m_buttons[0];
//    auto& uncovering_level_upgrades = m_buttons[1];
//    auto& uncovering_speed_upgrades = m_buttons[2];
//    auto& long_distance_communication_upgrade = m_buttons[3];
//
//    auto& params = ParamsManager::Instance().params.guns.radar;
//    UpgradeButton radius_upgrades_I(TextureID::RadarUpgradeRadius, params.radius_upgrades[1].cost, radar.radius_upgrade, AchievementSystem::Instance().radar_upgrades.radius_upgrades, 1, "Радиус обнаружения I");
//    UpgradeButton radius_upgrades_II(TextureID::RadarUpgradeRadius, params.radius_upgrades[2].cost, radar.radius_upgrade, AchievementSystem::Instance().radar_upgrades.radius_upgrades, 2, "Радиус обнаружения II");
//    UpgradeButton radius_upgrades_III(TextureID::RadarUpgradeRadius, params.radius_upgrades[3].cost, radar.radius_upgrade, AchievementSystem::Instance().radar_upgrades.radius_upgrades, 3, "Радиус обнаружения III");
//    radius_upgrades.push_back(std::move(radius_upgrades_I));
//    radius_upgrades.push_back(std::move(radius_upgrades_II));
//    radius_upgrades.push_back(std::move(radius_upgrades_III));
//    for (size_t i = 0; i < 3; ++i) {
//        radius_upgrades[i].on_mouse_enter = [&, i]() {
//            auto& up = radius_upgrades[i];
//            InfoPanel panel;
//            panel.set_name("Радиус обнаружения " + std::string(i + 1, 'I'));
//            panel.set_description("Радиус обнаружения противников увеличиться");
//            panel.add_char("Радиус обнаружения", params.radius_upgrades[i + 1].radius);
//            panel.create();
//            info->removeAllWidgets();
//            info->add(panel.content);
//        };
//    }
//    UpgradeButton uncovering_level_upgrade_I(TextureID::RadarUpgradeInterferrenceSuppression, params.uncovering_level_upgrades[1].cost, radar.uncovering_level_upgrade, AchievementSystem::Instance().radar_upgrades.uncovering_level_upgrades, 1, "Система радиоподавления I");
//    UpgradeButton uncovering_level_upgrade_II(TextureID::RadarUpgradeInterferrenceSuppression, params.uncovering_level_upgrades[2].cost, radar.uncovering_level_upgrade, AchievementSystem::Instance().radar_upgrades.uncovering_level_upgrades, 2, "Система радиоподавления II");
//    UpgradeButton uncovering_level_upgrade_III(TextureID::RadarUpgradeInterferrenceSuppression, params.uncovering_level_upgrades[3].cost, radar.uncovering_level_upgrade, AchievementSystem::Instance().radar_upgrades.uncovering_level_upgrades, 3, "Система радиоподавления III");
//    uncovering_level_upgrades.push_back(std::move(uncovering_level_upgrade_I));
//    uncovering_level_upgrades.push_back(std::move(uncovering_level_upgrade_II));
//    uncovering_level_upgrades.push_back(std::move(uncovering_level_upgrade_III));
//    for (size_t i = 0; i < 3; ++i) {
//        uncovering_level_upgrades[i].on_mouse_enter = [&, i]() {
//            auto& up = uncovering_level_upgrades[i];
//            InfoPanel panel;
//            panel.set_name("Система радиоподавления " + std::string(i + 1, 'I'));
//            panel.set_description("Радар будет обнаруживать противников с более высоким уровнем маскировки.");
//            panel.add_char("Уровень маскировки", params.uncovering_level_upgrades[i + 1].uncovering_level);
//            panel.create();
//            info->removeAllWidgets();
//            info->add(panel.content);
//        };
//    }
//    UpgradeButton uncovering_speed_upgrade_I(TextureID::RadarUpgradeUncoverSpeed, params.uncovering_speed_upgrades[1].cost, radar.uncovering_speed_upgrade, AchievementSystem::Instance().radar_upgrades.uncovering_speed_upgrades, 1, "Скорость обнаружения I");
//    UpgradeButton uncovering_speed_upgrade_II(TextureID::RadarUpgradeUncoverSpeed, params.uncovering_speed_upgrades[2].cost, radar.uncovering_speed_upgrade, AchievementSystem::Instance().radar_upgrades.uncovering_speed_upgrades, 2, "Скорость обнаружения II");
//    UpgradeButton uncovering_speed_upgrade_III(TextureID::RadarUpgradeUncoverSpeed, params.uncovering_speed_upgrades[3].cost, radar.uncovering_speed_upgrade, AchievementSystem::Instance().radar_upgrades.uncovering_speed_upgrades, 3, "Скорость обнаружения III");
//    uncovering_speed_upgrades.push_back(std::move(uncovering_speed_upgrade_I));
//    uncovering_speed_upgrades.push_back(std::move(uncovering_speed_upgrade_II));
//    uncovering_speed_upgrades.push_back(std::move(uncovering_speed_upgrade_III));
//    for (size_t i = 0; i < 3; ++i) {
//        uncovering_speed_upgrades[i].on_mouse_enter = [&, i]() {
//            auto& up = uncovering_speed_upgrades[i];
//            InfoPanel panel;
//            panel.set_name("Скорость обнаружения " + std::string(i + 1, 'I'));
//            panel.set_description("Время выбора цели (наведения) и время её обнаружения уменьшиться.");
//            panel.add_char("Время наведения", params.uncovering_speed_upgrades[i + 1].aiming_time);
//            panel.add_char("Время обнаружения цели", params.uncovering_speed_upgrades[i + 1].uncover_time);
//            panel.create();
//            info->removeAllWidgets();
//            info->add(panel.content);
//        };
//    }
//
//    UpgradeButton long_distace_communication_upgrade(TextureID::RadarUpgradeLongDistanceCommunication, params.long_distance_communication_upgrade_cost, radar.long_distance_communication_upgrade, AchievementSystem::Instance().radar_upgrades.long_distance_communication_upgrade, 1, "Дальняя связь");
//    long_distace_communication_upgrade.on_mouse_enter = [&] () {
//        InfoPanel panel;
//        panel.set_name("Дальняя связь");
//        panel.set_description("Радар получит возможность быть частью сети, образованной радиовышками.");
//        panel.create();
//        info->removeAllWidgets();
//        info->add(panel.content);
//    };
//    long_distance_communication_upgrade.push_back(std::move(long_distace_communication_upgrade));
//
//
//    tgui::Grid::Ptr grid = tgui::Grid::create();
//
//    for (int cat = 0; cat < m_buttons.size(); ++cat)
//        for (int up = 0; up < m_buttons[cat].size(); ++up) {
//            grid->addWidget(m_buttons[cat][up].m_group, cat, up);
//            float size = GameState::Instance().window.getSize().y * 0.1;
//            m_buttons[cat][up].m_group->setSize(size, size);
//        }
//    panel->add(grid, "Grid");
//
//    m_compute_cost = [&]() {
//        int cost = params.cost;
//        for (int i = 1; i <= radar.radius_upgrade; ++i) cost += params.radius_upgrades[i].cost;
//        for (int i = 1; i <= radar.uncovering_level_upgrade; ++i) cost += params.uncovering_level_upgrades[i].cost;
//        for (int i = 1; i <= radar.uncovering_speed_upgrade; ++i) cost += params.uncovering_speed_upgrades[i].cost;
//        if (radar.long_distance_communication_upgrade)
//            cost += params.long_distance_communication_upgrade_cost;
//        return cost;
//    };
//    auto sell_button = create_sell_button(radar.x_id, radar.y_id);
//    sell_button->setPosition(0, "Grid.bottom");
//    panel->add(sell_button, "SellButton");
//
//    info = tgui::Group::create();
//    info->setPosition(0, "SellButton.bottom");
//    panel->add(info);
//}
//
//void UpgradePanelCreator::visit(RadioMast& radio_tower) {
//    auto& net_manager = NetManager::Instance();
//    auto& net = net_manager.get_net_by_radiotower({ radio_tower.x_id, radio_tower.y_id });
//
//    reset();
//
//    InfoPanel info_panel;
//    info_panel.set_name(to_string(BuildingType::RadioMast));
//    info_panel.set_description("Параметры сети");
//    info_panel.add_char("число радиовышек в сети", std::to_string(net.radio_towers.size()));
//    info_panel.add_char("число радаров в сети", std::to_string(net.radars.size()));
//    info_panel.create(false);
//    panel->removeAllWidgets();
//    panel->add(info_panel.content, "Content");
//
//
//    m_compute_cost = []() {
//        return ParamsManager::Instance().params.guns.radio_tower.cost;
//    };
//    tgui::Button::Ptr sell_button = create_sell_button(radio_tower.x_id, radio_tower.y_id);
//    sell_button->setPosition(0, "Content.top");
//    panel->add(sell_button, "SellButton");
//
//    info = tgui::Group::create();
//    info->setPosition(0, "SellButton.bottom");
//    panel->add(info);
//}
//
//void UpgradePanelCreator::update() {
//    for (auto& cat : m_buttons) {
//        for (auto& up : cat) {
//            up.update();
//        }
//    }
//}
//
//UpgradePanelCreator::~UpgradePanelCreator() {
//    reset();
//}
