#include "gui/upgrade_panel.h"
#include "tile_map.h"
#include <vector>
#include "guns/minigun.h"
#include "game_state.h"
#include "achievement_system.h"
#include "gui/info_panel.h"

UpgradeButton::UpgradeButton(
    TextureID icon,
    int cost,
    BuildingUpgrade& building_upgrade,
    int& achievement_system_upgrade,
    int goal_upgrade_value,
    const std::string name
):
    m_cost(cost), m_building_upgrade(building_upgrade), m_achievement_system_upgrade(achievement_system_upgrade),
    m_goal_upgrade_value(goal_upgrade_value), m_name(name),
    IconButton(icon, TextureID::UpgradeButtonBackground, TextureID::UpgradeButtonBackgroundCompleted) {
    connect();
    update();
}

UpgradeButton::UpgradeButton(UpgradeButton&& btn): IconButton(std::move(btn)),
    m_building_upgrade(btn.m_building_upgrade),
    m_achievement_system_upgrade(btn.m_achievement_system_upgrade),
    m_goal_upgrade_value(btn.m_goal_upgrade_value), m_name(btn.m_name), m_reason{btn.m_reason}
{
    m_cost = btn.m_cost;
    on_mouse_enter = btn.on_mouse_enter;
    on_mouse_leave = btn.on_mouse_leave;
    connect();
}

void UpgradeButton::connect() {
    m_button->onPress.disconnectAll();
    m_button->onPress.connect([&]() {
        if (m_state == State::Active) {
            m_building_upgrade = m_goal_upgrade_value;
            GameState::Instance().player_coins_add(-m_cost);
            GameState::Instance().update_upgrade_panel();
        }
    });
    m_button->onMouseEnter.disconnectAll();
    m_button->onMouseEnter.connect([&]() {
        GameState::Instance().set_tooltip_content(
           "<b>"+  m_name + "</b>\n<color=#ffd303>Стоимость: " + std::to_string(m_cost) + "</color>" + m_reason
            , {1.,0.});
        if (on_mouse_enter) on_mouse_enter();
    });
    m_button->onMouseLeave.disconnectAll();
    m_button->onMouseLeave.connect([&]() {
        GameState::Instance().set_tooltip_content("");
        if (on_mouse_leave) on_mouse_leave();
    });
}



void UpgradeButton::update() {
    m_reason = "";
    if (m_building_upgrade >= m_goal_upgrade_value) {
        set_state(State::Selected);
    }
    else { // еще не достигли нужного уровня
        bool locked = false;
        if (m_goal_upgrade_value > m_building_upgrade + 1) { // закрыто, так как нужно купить предыдущий уровень
            locked = true;
            m_reason = "\n<color=#ff0000>Закрыто: </color> требуется предыдущее улучшение.";
        }
        if (m_achievement_system_upgrade < m_goal_upgrade_value) {
            m_reason += "\n<color=#ff0000>Закрыто: </color> " + AchievementSystem::Instance().get_upgrade_unlock_condition_description(&m_achievement_system_upgrade, m_goal_upgrade_value) + "."; 
            locked = true;
        }
        if (locked) {
            set_state(State::Locked);
        }
        else { // открыто
            int current_coins_count = GameState::Instance().get_player_coins();
            if (current_coins_count >= m_cost)
                set_state(State::Active);
            else
                set_state(State::Disabled);
        }
    }
}




UpgradePanelCreator::UpgradePanelCreator() {
    panel = tgui::Group::create();
    panel->setSize("100%", "100%");
}

void UpgradePanelCreator::reset() {
    m_buttons.clear();
    panel->removeAllWidgets();
    //panel->onSizeChange.disconnectAll();
    for (auto& action : m_clear_actions)
        action();
    m_clear_actions.clear();
}

void UpgradePanelCreator::visit(MiniGun& minigun) {
    reset();

    m_buttons.resize(3);
    auto& shells_upgrades = m_buttons[0];
    auto& cooling_upgrades = m_buttons[1];
    auto& lubricant_upgrades = m_buttons[2];

    auto& params = ParamsManager::Instance().params.guns.minigun;
    UpgradeButton shells_upgrade_I(TextureID::MinigunShellsUpgradeI, 1000, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 1, "Бронебойные снаряды I");
    UpgradeButton shells_upgrade_II(TextureID::MinigunShellsUpgradeII, 2000, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 2, "Бронебойные снаряды II");
    UpgradeButton shells_upgrade_III(TextureID::MinigunShellsUpgradeIII, 3000, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 3, "Бронебойные снаряды III");
    shells_upgrades.push_back(std::move(shells_upgrade_I));
    shells_upgrades.push_back(std::move(shells_upgrade_II));
    shells_upgrades.push_back(std::move(shells_upgrade_III));
    for (size_t i = 0; i < 3;++i) {
        shells_upgrades[i].on_mouse_enter = [&, i]() {
            auto& up = shells_upgrades[i];
            InfoPanel panel;
            panel.set_name("Бронебойные снаряды " + std::string(i + 1, 'I'));
            panel.set_description("Пулемет получит снаряды повышенной бронепробиваемости.");
            panel.add_char("бронепробиваемость при минимальном нагреве", params.penetration_upgrades[i + 1].min_armor_penetration_level);
            panel.add_char("бронепробиваемость при максимальном нагреве", params.penetration_upgrades[i + 1].max_armor_penetration_level);
            panel.add_char("минимальный урон", params.penetration_upgrades[i + 1].min_damage);
            panel.add_char("максимальный урон", params.penetration_upgrades[i + 1].max_damage);
            panel.create();
            info->removeAllWidgets();
            info->add(panel.content);
        };
    }
    UpgradeButton cooling_upgrade_I(TextureID::MinigunCoolingUpgradeI, 1000, minigun.m_cooling_upgrade, AchievementSystem::Instance().minigun_upgrades.cooling_upgrade, 1, "Система охлаждения I");
    UpgradeButton cooling_upgrade_II(TextureID::MinigunCoolingUpgradeI, 2000, minigun.m_cooling_upgrade, AchievementSystem::Instance().minigun_upgrades.cooling_upgrade, 2, "Система охлаждения II");
    UpgradeButton cooling_upgrade_III(TextureID::MinigunCoolingUpgradeI, 3000, minigun.m_cooling_upgrade, AchievementSystem::Instance().minigun_upgrades.cooling_upgrade, 3, "Система охлаждения III");
    cooling_upgrades.push_back(std::move(cooling_upgrade_I));
    cooling_upgrades.push_back(std::move(cooling_upgrade_II));
    cooling_upgrades.push_back(std::move(cooling_upgrade_III));
    for (size_t i = 0; i < 3; ++i) {
        cooling_upgrades[i].on_mouse_enter = [&, i]() {
            auto& up = cooling_upgrades[i];
            InfoPanel panel;
            panel.set_name("Система охлаждения " + std::string(i + 1, 'I'));
            panel.set_description("Пулемет получит улучшенную систему охлаждения, продливающую время работы при критическом нагреве, а также увеличивающую скорость охлаждения.");
            panel.add_char("время работы при критическом перегреве", params.cooling_upgrades[i + 1].critical_temperature_work_duration);
            panel.add_char("время полного охлаждения", params.cooling_upgrades[i + 1].cooling_time);
            panel.add_char("время на охлаждение после перегрева", params.cooling_upgrades[i + 1].cooldown_duration);
            panel.create();
            info->removeAllWidgets();
            info->add(panel.content);
        };
    }
    UpgradeButton lubricant_upgrade_I(TextureID::MinigunLubricantUpgradeI, 1000, minigun.m_lubricant_upgrade, AchievementSystem::Instance().minigun_upgrades.lubricant_update, 1, "Смазка I");
    UpgradeButton lubricant_upgrade_II(TextureID::MinigunLubricantUpgradeI, 2000, minigun.m_lubricant_upgrade, AchievementSystem::Instance().minigun_upgrades.lubricant_update, 2, "Смазка II");
    UpgradeButton lubricant_upgrade_III(TextureID::MinigunLubricantUpgradeI, 3000, minigun.m_lubricant_upgrade, AchievementSystem::Instance().minigun_upgrades.lubricant_update, 3, "Смазка III");
    lubricant_upgrades.push_back(std::move(lubricant_upgrade_I));
    lubricant_upgrades.push_back(std::move(lubricant_upgrade_II));
    lubricant_upgrades.push_back(std::move(lubricant_upgrade_III));
    for (size_t i = 0; i < 3; ++i) {
        lubricant_upgrades[i].on_mouse_enter = [&, i]() {
            auto& up = lubricant_upgrades[i];
            InfoPanel panel;
            panel.set_name("Смазка " + std::string(i + 1, 'I'));
            panel.set_description("Пулемет получит улучшенную смачную систему, что позволит ему набирать максимальную скорость вращения барабана быстрее. При этом соответсвие скорости и нагрева останется прежним.");
            panel.add_char("Время нагрева до максимальной температуры", params.lubricant_upgrades[i + 1].heating_time);
            panel.create();
            info->removeAllWidgets();
            info->add(panel.content);
        };
    }

    tgui::Grid::Ptr grid = tgui::Grid::create();
    
    for (int cat = 0; cat < m_buttons.size(); ++cat)
        for (int up = 0; up < m_buttons[cat].size(); ++up) {
            grid->addWidget(m_buttons[cat][up].m_group, cat, up);
            float size = GameState::Instance().window.getSize().y * 0.1;
            m_buttons[cat][up].m_group->setSize(size, size);
        }
    /*grid->setWidth("100%");
    grid->setHeight("100%");*/
    /*grid->onSizeChange.connect([=]() {
        float y = 0;
        float size = panel->getSize().x / 3.;
        for (auto& btns : m_buttons) {
            for (auto& btn : btns)
                btn.m_group->setSize({ size,size });
        }   
    });*/
    panel->add(grid, "Grid");
    info = tgui::Group::create();
    info->setPosition(0, "Grid.bottom");
    panel->add(info);
    
}

template<typename Params, typename Building>
tgui::Group::Ptr create_panel_for_building_with_health(UpgradePanelCreator* creator, BuildingType type, Params& params, Building& building) {
    auto label = tgui::RichTextLabel::create();
    auto update = [&, label = label.get()]() {
        if (building.get_health() <= 0) {
            creator->reset();
            return;
        }
        label->setText("<b>" + to_string(BuildingType::Spikes) + "</b>\nПрочность: " + std::to_string(building.get_health()));
    };
    update();
    label->getRenderer()->setTextColor(sf::Color::White);
    building.set_health_changed_callback(update);
    creator->m_clear_actions.push_back([&building]() {
        building.set_health_changed_callback({});
    });

    auto description = tgui::Label::create();
    description->setVisible(false);
    description->getRenderer()->setTextColor(sf::Color::White);
    auto button = tgui::Button::create("Укрепить");
    button->onMouseEnter.connect([&params, description = description.get()]() {
        description->setVisible(true);
        description->setText("Увеличит прочность на " + std::to_string(params.health)  + " единиц.");
        GameState::Instance().set_tooltip_content("<color=#ffd303>Стоимость: " + std::to_string(params.cost) + "</color>", { 1.,0. });
    });
    button->onMouseLeave.connect([description = description.get()]() {
        description->setVisible(false);
        GameState::Instance().set_tooltip_content("");
    });
    button->onClick.connect([&params, &building]() {
        int coins = GameState::Instance().get_player_coins();
        if (coins >= params.cost) {
            building.set_health(building.get_health() + 5);
            GameState::Instance().player_coins_add(-params.cost);
        }
    });

    auto auto_repair_button = tgui::ToggleButton::create("Автовосстановление");
    auto_repair_button->onMouseEnter.connect([type, &params, description = description.get()]() {
        description->setVisible(true);
        description->setText("Включает автовосстановление. При достижении нулевой прочности и наличии достаточных средств, " + to_string(type) + " автоматически восстаноят 5 единиц прочности.");
    });
    auto_repair_button->onMouseLeave.connect([description = description.get()]() {
        description->setVisible(false);
    });
    auto_repair_button->onToggle([&building](bool isDown) {
        building.auto_repair = isDown;
    });
    auto_repair_button->setDown(building.auto_repair);



    auto group = tgui::Group::create();
    group->setSize("100%", "100%");

    group->add(label, "Header");
    button->setPosition(0, "Header.bottom");
    group->add(button, "Endurance");
    auto_repair_button->setPosition(0, "Endurance.bottom");
    group->add(auto_repair_button, "Repair");
    description->setPosition(0, "Repair.bottom");
    group->add(description);
    group->onSizeChange.connect([group = group.get(), description = description.get()]() {
        description->setMaximumTextWidth(group->getSize().x);
    });
    return group;
}

void UpgradePanelCreator::visit(Spikes& spikes) {
    // TODO этот код почему-то вызывает ошибку при закрытии программы
    reset();   
    panel->add(create_panel_for_building_with_health(this, BuildingType::Spikes, ParamsManager::Instance().params.guns.spikes, spikes));
}

void UpgradePanelCreator::visit(Hedgehog& headgehogs) {
    // TODO этот код почему-то вызывает ошибку при закрытии программы
    reset();
    panel->add(create_panel_for_building_with_health(this, BuildingType::Hedgehogs, ParamsManager::Instance().params.guns.hedgehog, headgehogs));
}

void UpgradePanelCreator::visit(AntitankGun& antitank_gun) {
    reset();
}

void UpgradePanelCreator::visit(TwinGun& twingun) {
    reset();
}

void UpgradePanelCreator::visit(Mine& mine) {
    reset();
}

void UpgradePanelCreator::update() {
    for (auto& cat : m_buttons) {
        for (auto& up : cat) {
            up.update();
        }
    }
}

UpgradePanelCreator::~UpgradePanelCreator() {
    reset();
}
