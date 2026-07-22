#include "gui/upgrade_panel.h"
#include "gui/label.h"
#include "achievement_system.h"
#include "game_state.h"
#include "gui/tooltip.h"
#include "gui/switch.h"
#include "gui/text_button.h"

#include "guns/spikes.h"
#include "guns/hedgehog.h"
#include "guns/radio_mast.h"

#include <sstream>

struct Stringifier: public IStringifier {
public:
    Comparation compare(const std::string& prop, float current, float goal) override {
        Comparation comp;
        comp.current_value = to_string_2(current);
        comp.goal_value = to_string_2(goal);
        comp.prop = prop;
        return comp;
    };
    Comparation compare(const std::string& prop, int current, int goal) override {
        Comparation comp;
        comp.current_value = std::to_string(current);
        comp.goal_value = std::to_string(goal);
        comp.prop = prop;
        return comp;
    };
private:
    std::string to_string_2(double number) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << number;
        return out.str();
    }
};



UpgradeButton::UpgradeButton(
    Widget* ui,
    UpgradePanel* upgrade_panel,
    UpgradeButton* prev_upgrade_button,
    IBuilding* building,
    Upgrade* upgrade,
    int level,
    TextureID upgrade_icon):
    m_ui{ui},
    m_upgrade_panel{upgrade_panel},
    m_prev_upgrade_button{prev_upgrade_button},
    m_building{building},
    m_upgrade{upgrade},
    m_level{level},
    m_upgrade_icon{upgrade_icon}
{
    m_state = m_upgrade->get_current_upgrate(m_building) >= m_level ? State::BOUGTH : State::UNDISCOVERED;
    set_state(m_state);

    m_button = Button::LEFT | Button::RIGHT;
    capture_mode = true;
    unhover_on_pressed = false;
    set_on_hovered([this]() {
        create_tooltip_smart();
    });

    set_on_unhovered([this]() {
        delete_tooltip_smart();
    });

    set_on_mouse_moved([this]() {
        if (m_tooltip)
            m_tooltip->invalidate(Property::POSITION);
    });

    set_on_released([this](Button::Type button) {
        if (button == Button::LEFT) { //попытка купить
            if (m_state == State::ACTIVE) { //нужно проверить, пока мышь была зажата, деньги могли уменьшиться (автовостановление шипов)
                m_upgrade->upgrade(m_building, m_level); //апгрейдим до нужного уровня.
                set_state(State::BOUGTH);
                GameState::Instance().player_coins_add(-m_upgrade->cost(m_level));
            }
        }
        else { //правая кнопка => выделим
            m_upgrade_panel->capture(this);
        }
    });
}

void UpgradeButton::update(int player_coins) {
    if (m_state == State::BOUGTH)
        return;
    if (m_state == State::UNDISCOVERED) {
        if ((m_prev_upgrade_button && m_prev_upgrade_button->get_state() != State::BOUGTH) || m_upgrade->available_level < m_level)
            return; //остается UNDISCOVERED
    }
    State state = player_coins >= m_upgrade->cost(m_level) ? State::ACTIVE : State::NOT_ENOGTH_MONEY;
    if (m_tooltip && m_state != state) { //состояние изменилось => нужно изменить tooltip.
        delete_tooltip_smart();
        create_tooltip_smart();
    }
    set_state(state);
}


void UpgradeButton::create_tooltip_smart() {
    auto [tooltip, label] = ::create_tooltip(Anchor::TOP | Anchor::RIGHT);
    m_tooltip = tooltip.get();

    label->add_line(m_upgrade->get_name(m_level), sf::Color::White, sf::Text::Style::Bold);
    switch (m_state) {
    case UpgradeButton::State::ACTIVE:
        label->add_line("Стоимость: " + std::to_string(m_upgrade->cost(m_level)), Label::coins_color);
        label->add_line("ПКМ: купить.", sf::Color::White, sf::Text::Italic);
        break;
    case UpgradeButton::State::BOUGTH:
        label->add_line("Установлено");
        break;
    case UpgradeButton::State::NOT_ENOGTH_MONEY:
        label->add_line("Стоимость: " + std::to_string(m_upgrade->cost(m_level)), Label::coins_color);
        label->add_line("Недостаточно средств.", Label::prohibited_color);
        break;
    case UpgradeButton::State::UNDISCOVERED:
        if (m_prev_upgrade_button && m_prev_upgrade_button->get_state() != State::BOUGTH)
            label->add_line("Закрыто: необходимо купить предыдущее улучшение.", Label::prohibited_color);
        else
            label->add_line("Закрыто: " + m_upgrade->get_unlock_condition_description(m_level), Label::prohibited_color);
        break;
    default:
        break;
    }
    label->add_line("ЛКМ: выделить апгрейд для просмотра детальной информации.", sf::Color::White, sf::Text::Italic);

    m_ui->add_widget_smart(std::move(tooltip));
}

void UpgradeButton::delete_tooltip_smart() {
    if (m_tooltip) {
        m_ui->delete_widget_smart(m_tooltip);
        m_tooltip = nullptr;
    }
}


void UpgradeButton::set_state(State state) {
    m_state = state;
    switch (state) {
    case UpgradeButton::State::ACTIVE:
        layers = { TextureID::UpgradeButtonBackground, m_upgrade_icon };
        grayscale = false;
        enabled(Button::LEFT);
        break;
    case UpgradeButton::State::BOUGTH:
        layers = { TextureID::UpgradeButtonBackgroundCompleted, m_upgrade_icon };
        grayscale = false;
        enabled(Button::LEFT);
        break;
    case UpgradeButton::State::NOT_ENOGTH_MONEY:
        layers = { TextureID::UpgradeButtonBackground, m_upgrade_icon };
        enabled(~Button::LEFT);
        grayscale = true;
        break;
    case UpgradeButton::State::UNDISCOVERED:
        layers = { TextureID::UpgradeButtonBackground, m_upgrade_icon, TextureID::UpgradeLock };
        grayscale = true;
        enabled(~Button::LEFT);
        break;
    default:
        break;
    }
}

void UpgradeButton::set_capture(bool capture) {
    if (m_captured != capture) {
        m_captured = capture;
        if (m_captured) {
            auto capture_icon = Icon::create(TextureID::UpgradeButtonCapture);
            m_capture_icon = capture_icon.get();
            add_widget_deffered(std::move(capture_icon));
            m_capture_icon->size_inherited(this);
        }
        else {
            delete_widget_deffered(m_capture_icon);
            m_capture_icon = nullptr;
        }
    }
}


UpgradePanel::UpgradePanel(Widget* tile_size_reference, Widget* height_reference):
    TiledPanel(TiledPanel::Type::Blueprint, tile_size_reference),
    m_tile_size_reference(tile_size_reference), m_height_reference(height_reference)
{
    DEBUG_TAG(content_widget, "content_widget");
    clear_rules(Property::LAYOUT);

    add_rule(Property::SIZE, [this](Layout& layout) {
        m_cached_tile_size = m_tile_size_reference->layout.width;
        m_cached_tiles_count.x = std::ceil(std::max<float>((content_widget->layout.width + m_cached_tile_size) / m_cached_tile_size, 2));
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

UpgradePanel::~UpgradePanel() {
    for (auto& action : m_on_kill_actions)
        action();
}

void UpgradePanel::capture(UpgradeButton* button) {
    for (UpgradeButton* up_button : m_upgrade_buttons) {
        if (up_button == button) {
            if (!up_button->is_captured()) {
                create_info_panel_for_button(up_button);
                up_button->set_capture(true);
            }
        }
        else
            up_button->set_capture(false);
    }
}

void UpgradePanel::create_info_panel_for_button(UpgradeButton* button) {
    GUI::Instance().add_deffered_command([this, button]() {
        m_upgrade_info_widget->clear_rules(Property::SIZE); //удалим правило вычисления размера (нужно потому что используем vbox)
        m_upgrade_info_widget->delete_all_widgets(); 

        Label* description = (Label*)m_upgrade_info_widget->add_widget(Label::create()); //label для общего описания апгрейда
        description->add_line(button->get_upgrate()->name + " " + std::string(button->get_level(), 'I'), sf::Color::White, sf::Text::Style::Bold);
        description->add_line(button->get_upgrate()->general_description, Label::blueprint_color);
        DEBUG_TAG(description, "description");

      
        std::vector<IStringifier::Comparation> comparations = button->get_upgrate()->compare(Stringifier(), button->get_building(), button->get_level());
        if (!comparations.empty()) {
            Widget* prop_table = m_upgrade_info_widget->add_widget(Widget::create());
            DEBUG_TAG(prop_table, "prop_table");
            std::vector<std::vector<Widget*>> prop_table_labels;
            for (auto& comp : comparations) {
                auto& line = prop_table_labels.emplace_back();
                for (size_t i = 0; i < 4; ++i) {
                    line.push_back(prop_table->add_widget(Label::create(true)));
                    DEBUG_TAG(line.back(), "elem[" + std::to_string(prop_table_labels.size() - 1) + "][" + std::to_string(i) + "]");
                }
                static_cast<Label*>(line[0])->add_text(comp.prop, Label::blueprint_color);
                static_cast<Label*>(line[1])->add_text(comp.current_value, Label::coins_color);
                static_cast<Label*>(line[2])->add_text("->", Label::coins_color);
                static_cast<Label*>(line[3])->add_text(comp.goal_value, Label::coins_color);
            }
            GridOptions options;
            options.alignment = Anchor::BOTTOM | Anchor::LEFT;
            prop_table->grid(prop_table_labels, {});
            m_upgrade_info_widget->vbox({ description, prop_table });
            description->property_equal(Property::WIDTH, false, prop_table, Property::WIDTH, false, {});
        }
        else {
            description->property_equal(Property::WIDTH, false, button->get_parent()->get_parent(), Property::WIDTH, false, {});
            m_upgrade_info_widget->size_include(description);
        }
    });
}

void UpgradePanel::update(int player_coins) {
    for (UpgradeButton* up_button : m_upgrade_buttons)
        up_button->update(player_coins);
    for (auto& update_callback : m_update_callbacks)
        update_callback(player_coins);
}

Query UpgradePanel::on_event(EventContext event_context) {
    return Query{ Query::PROCESSED };
}

Widget* UpgradePanel::create_buttons_for_upgrade(Widget* parent, IBuilding* building, Upgrade* upgrade, const std::vector<TextureID>& upgrade_buttons_icons) {
    Widget* upgrades_panel = parent->add_widget(Widget::create());
    std::vector<Widget*> upgrade_buttons;
    UpgradeButton* prev = nullptr;
    for (size_t i = 1; i <= upgrade->max_level; ++i) {
        UpgradeButton* button = upgrades_panel->add_widget(std::make_unique<UpgradeButton>(
            m_parent,
            this,
            prev,
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
        prev = button;
    }
    upgrades_panel->hbox(upgrade_buttons);
    return upgrades_panel;
}

// автоматически настроит margin
VHBoxOptions UpgradePanel::options_for_vhbox() {
    VHBoxOptions vbox_options;
    vbox_options.margin_source = 0.1;
    vbox_options.reference = m_tile_size_reference;
    vbox_options.margin_function = Margin::Source::FRACTION_OF_REFERENCE_HEIGHT;
    return vbox_options;
}

void UpgradePanel::visit(MiniGun& minigun) {
    Widget* content = create_header_and_content(BuildingType::Minigun);
    DEBUG_TAG(content, "content");

    auto& achievement_system = AchievementSystem::Instance();
    Widget* upgrade_buttons_panel = content->add_widget(Widget::create());
    DEBUG_TAG(upgrade_buttons_panel, "upgrade_buttons_panel");
    m_upgrade_info_widget = content->add_widget(Widget::create());
    DEBUG_TAG(m_upgrade_info_widget, "m_upgrade_info_widget");
    m_upgrade_info_widget->size_fixed(0, 0);

    Widget* penetration_upgrades_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &minigun, &achievement_system.minigun_penetration_upgrade, { TextureID::MinigunShellsUpgradeI, TextureID::MinigunShellsUpgradeII, TextureID::MinigunShellsUpgradeIII });
    Widget* cooling_upgrade_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &minigun, &achievement_system.minigun_cooling_upgrade, { TextureID::MinigunCoolingUpgradeI, TextureID::MinigunCoolingUpgradeI, TextureID::MinigunCoolingUpgradeI });
    Widget* lubricant_upgrade_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &minigun, &achievement_system.minigun_lubricant_upgrade, { TextureID::MinigunLubricantUpgradeI, TextureID::MinigunLubricantUpgradeI, TextureID::MinigunLubricantUpgradeI });
    upgrade_buttons_panel->vbox({ penetration_upgrades_panel, cooling_upgrade_panel, lubricant_upgrade_panel });

    VHBoxOptions options = options_for_vhbox();
    options.items = {
        {upgrade_buttons_panel, Anchor::RIGHT},
        {m_upgrade_info_widget, Anchor::LEFT}
    };
    content->vbox(options);
}

Widget* UpgradePanel::create_header_and_content(BuildingType type) {
    Widget* headline = content_widget->add_widget(Widget::create());
    DEBUG_TAG(headline, "headline");
        //HEADLINE
        Label* title = headline->add_widget(Label::create(true, 40));
        title->add_text(to_string(type), sf::Color::White, sf::Text::Bold);
        DEBUG_TAG(title, "title");
        //HRULE
        Widget* rule = headline->add_widget(Panel::create(sf::Color::White, sf::Color::Transparent, 0));
        DEBUG_TAG(rule, "rule");
        VHBoxOptions vbox_options = options_for_vhbox();
        vbox_options.items = { {title, Anchor::CENTER}, { rule, Anchor::CENTER} };
        headline->vbox(vbox_options);

    Widget* content = content_widget->add_widget(Widget::create());
    vbox_options.items = { {headline, Anchor::CENTER}, { content, Anchor::CENTER} };
    content_widget->vbox(vbox_options);

    rule->add_rule(Property::WIDTH, [content, title](Layout& layout) {
        layout.width = std::max<float>(content->layout.width, title->layout.width);
    }, { {content, Property::WIDTH}, {title, Property::WIDTH} });
    rule->property_equal(Property::HEIGHT, false, m_tile_size_reference, Property::HEIGHT, false, modifiers::Multiply(0.02));
    return content;
}

void UpgradePanel::create_panel_for_building_with_health(BuildingWithHealth* building, int enforce_cost, int repairing_hp) {
    Widget* content = create_header_and_content(building->type);
    DEBUG_TAG(content, "content");
    //INTERFACE
    Widget* interface = content->add_widget(Widget::create());
    DEBUG_TAG(interface, "interface");
        //HEALTH WIDGETS
        Label* health_panel_label = interface->add_widget(Label::create(true));
        DEBUG_TAG(health_panel_label, "health_panel_label");
        health_panel_label->add_text("Прочность: " + std::to_string(building->get_health()));
        TextButton* enforce_button = interface->add_widget(std::make_unique<TextButton>("Укрепить"));
        enforce_button->enable(enforce_cost <= GameState::Instance().get_player_coins());
        DEBUG_TAG(enforce_button, "enforce_button");
        enforce_button->property_equal(Property::HEIGHT, false, m_parent, Property::HEIGHT, false, modifiers::Multiply(0.05));
        //REPAIRING WIDGETS
        Switch* auto_repairing_switch = interface->add_widget(std::make_unique<Switch>(building->auto_repair, [building](bool autorepairing_enabled) {
            building->auto_repair = autorepairing_enabled;
        }));
        DEBUG_TAG(auto_repairing_switch, "auto_repairing_switch");
        auto_repairing_switch->property_equal(Property::HEIGHT, false, m_parent, Property::HEIGHT, false, modifiers::Multiply(0.05));
        Label* auto_repairing_label = interface->add_widget(Label::create(true));
        DEBUG_TAG(auto_repairing_label, "auto_repairing_label");
        auto_repairing_label->add_text("Автовосстановление");
    //INFO PANEL
    m_upgrade_info_widget = content->add_widget(Widget::create());
    DEBUG_TAG(m_upgrade_info_widget, "m_upgrade_info_widget");
    m_upgrade_info_widget->size_fixed(0, 0);

    GridOptions grid_options; grid_options.alignment = Anchor::Y_CENTER | Anchor::LEFT;
    grid_options.margin_source = 0.1;
    grid_options.reference = m_tile_size_reference;
    grid_options.margin_function = Margin::Source::FRACTION_OF_REFERENCE_HEIGHT;
    interface->grid({ { health_panel_label, enforce_button }, { auto_repairing_label, auto_repairing_switch} }, grid_options);

    VHBoxOptions vbox_options = options_for_vhbox();
    vbox_options.items = { {interface, Anchor::LEFT}, {m_upgrade_info_widget, Anchor::LEFT} };
    content->vbox(vbox_options);

    //EVENTS
    //INFOS
    auto unhover_event = [this]() {
        GUI::Instance().add_deffered_command([this]() {
            m_upgrade_info_widget->size_fixed(0, 0);
            m_upgrade_info_widget->delete_all_widgets();
        });
    };
    auto_repairing_switch->set_on_hovered([this, interface, type = building->type]() {
        GUI::Instance().add_deffered_command([this, interface, type]() {
            m_upgrade_info_widget->clear_rules(Property::SIZE);
            m_upgrade_info_widget->delete_all_widgets();
            Label* label = (Label*)m_upgrade_info_widget->add_widget(Label::create());
            label->add_text("Включает автовосстановление. При достижении нулевой прочности и наличии достаточных средств, " + to_string(type) + " автоматически восстановят 5 единиц прочности.", Label::blueprint_color);
            label->property_equal(Property::WIDTH, false, interface, Property::WIDTH, true, {});
            m_upgrade_info_widget->size_include(label);
        });
        return Query{ Query::PROCESSED };
    });
    auto_repairing_switch->set_on_unhovered(unhover_event);

    enforce_button->set_on_hovered([this, interface, repairing_hp, enforce_cost]() {
        GUI::Instance().add_deffered_command([this, interface, repairing_hp, enforce_cost]() {
            m_upgrade_info_widget->clear_rules(Property::SIZE);
            m_upgrade_info_widget->delete_all_widgets();
            Label* label = (Label*)m_upgrade_info_widget->add_widget(Label::create());
            label->add_line("Увеличивает прочность на " + std::to_string(repairing_hp) + " единиц.", Label::blueprint_color);
            label->add_text("Стоимость:" + std::to_string(enforce_cost), Label::coins_color);
            label->property_equal(Property::WIDTH, false, interface, Property::WIDTH, true, {});
            m_upgrade_info_widget->size_include(label);
        });
    });
    enforce_button->set_on_unhovered(unhover_event);
    //ENFORCE
    enforce_button->set_on_clicked([building, repairing_hp, enforce_cost]() {
        building->set_health(building->get_health() + repairing_hp);
        GameState::Instance().player_coins_add(-enforce_cost);
    });

    //CALLBACK ON HEALTH CHANGED
    building->set_health_changed_callback([building, health_panel_label]() {
        health_panel_label->clear();
        health_panel_label->add_text("Прочность: " + std::to_string(building->get_health()));
        if (building->get_health() <= 0) {
            GameState::Instance().close_upgrade_panel();
        }
    });
    m_on_kill_actions.push_back([building]() {
        building->set_health_changed_callback({}); //отписываемся.
    });
    m_update_callbacks.push_back([enforce_button, enforce_cost](int coins) {
        enforce_button->enable(coins >= enforce_cost);
    });
}

void UpgradePanel::visit(Spikes& spikes) {
    auto& params = ParamsManager::Instance().params.guns.spikes;
    create_panel_for_building_with_health(&spikes, params.cost, params.health);
}

void UpgradePanel::visit(Hedgehog& headgehogs) {
    auto& params = ParamsManager::Instance().params.guns.hedgehog;
    create_panel_for_building_with_health(&headgehogs, params.cost, params.health);
}

void UpgradePanel::visit(AntitankGun& antitank_gun) {

}

void UpgradePanel::visit(TwinGun& twingun) {

}

void UpgradePanel::visit(Mine& mine) {

}

void UpgradePanel::visit(Radar& radar) {
    Widget* content = create_header_and_content(BuildingType::Radar);
    DEBUG_TAG(content, "content");

    auto& achievement_system = AchievementSystem::Instance();
    Widget* upgrade_buttons_panel = content->add_widget(Widget::create());
    DEBUG_TAG(upgrade_buttons_panel, "upgrade_buttons_panel");
    m_upgrade_info_widget = content->add_widget(Widget::create());
    DEBUG_TAG(m_upgrade_info_widget, "m_upgrade_info_widget");
    m_upgrade_info_widget->size_fixed(0, 0);

    Widget* radius_upgrades_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &radar, &achievement_system.radar_radius_upgrade, { TextureID::RadarRadiusUpgrade, TextureID::RadarRadiusUpgrade, TextureID::RadarRadiusUpgrade });
    Widget* uncovering_level_upgrades_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &radar, &achievement_system.radar_uncovering_level_upgrade, { TextureID::RadarUncoveringLevelUpgrade, TextureID::RadarUncoveringLevelUpgrade, TextureID::RadarUncoveringLevelUpgrade });
    Widget* uncovering_speed_upgrades_panel = create_buttons_for_upgrade(upgrade_buttons_panel, &radar, &achievement_system.radar_uncovering_speed_upgrade, { TextureID::RadarUncoveringSpeedUpgrade, TextureID::RadarUncoveringSpeedUpgrade, TextureID::RadarUncoveringSpeedUpgrade });
    Widget* long_distance_communication_upgrade = create_buttons_for_upgrade(upgrade_buttons_panel, &radar, &achievement_system.radar_long_distance_communication_upgrade, { TextureID::RadarLongDistanceCommunicationUpgrade });
    upgrade_buttons_panel->vbox({ radius_upgrades_panel, uncovering_level_upgrades_panel, uncovering_speed_upgrades_panel, long_distance_communication_upgrade });

    VHBoxOptions options = options_for_vhbox();
    options.items = {
        {upgrade_buttons_panel, Anchor::RIGHT},
        {m_upgrade_info_widget, Anchor::LEFT}
    };
    content->vbox(options);
}


void UpgradePanel::visit(RadioMast& radio_tower) {
    Widget* content = create_header_and_content(BuildingType::RadioMast);
    DEBUG_TAG(content, "content");

    auto& net_manager = NetManager::Instance();
    auto& net = net_manager.get_net_by_radiotower({ radio_tower.x_id, radio_tower.y_id });

    Label* headline = content->add_widget(Label::create(true));
    headline->add_text("Параметры сети");

    Widget* props_grid = content->add_widget(Widget::create());


    auto make_prop = [](Widget* parent, std::string name, auto value)->std::vector<Widget*> {
        Label* label = parent->add_widget(Label::create(true));
        label->add_text(name, Label::blueprint_color);
        Label* value_widget = parent->add_widget(Label::create(true));
        value_widget->add_text(std::to_string(value), Label::coins_color);
        return { label, value_widget };
    };
    std::vector<std::vector<Widget*>> props;
    props.push_back(make_prop(props_grid, "число радиовышек в сети", net.radio_towers.size()));
    props.push_back(make_prop(props_grid, "число радаров в сети", net.radars.size()));
    GridOptions grid_options;
    grid_options.alignment = Anchor::LEFT | Anchor::BOTTOM;
    static_cast<Margin&>(grid_options) = options_for_vhbox();
    props_grid->grid(props, grid_options);
    content->vbox({ headline , props_grid });
}
