#include "gui/upgrade_panel.h"
#include "tile_map.h"
#include <vector>
#include "guns/minigun.h"
#include "game_state.h"
#include "achievement_system.h"

UpgradeButton::UpgradeButton(
    TextureID icon,
    int cost,
    int& building_upgrade,
    int& achievement_system_upgrade,
    int goal_upgrade_value,
    const std::string name
):
    m_cost(cost), m_building_upgrade(building_upgrade), m_achievement_system_upgrade(achievement_system_upgrade),
    m_goal_upgrade_value(goal_upgrade_value), m_name(name),
    IconButton(icon, TextureID::UpgradeButtonBackground, TextureID::UpgradeButtonBackgroundCompleted) {
    connect();
    if (building_upgrade == goal_upgrade_value)
        set_state(State::Selected);
    else if (achievement_system_upgrade >= goal_upgrade_value) {
        set_state(State::Active);
        coins_update(GameState::Instance().get_player_coins());
    }
}

UpgradeButton::UpgradeButton(UpgradeButton&& btn): IconButton(std::move(btn)),
    m_building_upgrade(btn.m_building_upgrade),
    m_achievement_system_upgrade(btn.m_achievement_system_upgrade),
    m_goal_upgrade_value(btn.m_goal_upgrade_value), m_name(btn.m_name)
{
    m_cost = btn.m_cost;
    on_upgrade = btn.on_upgrade;
    unlock_condition = btn.unlock_condition;
    connect();
}

void UpgradeButton::connect() {
    m_button->onPress.disconnectAll();
    m_button->onPress.connect([&]() {
        if (m_state == State::Active) {
            m_building_upgrade = m_goal_upgrade_value;
            GameState::Instance().player_coins_add(-m_cost);
            set_state(State::Selected);
        }
    });
    m_button->onMouseEnter.disconnectAll();
    m_button->onMouseEnter.connect([&]() {
        GameState::Instance().set_tooltip_content(
           "<b>"+  m_name + "</b>\n<color=#ffd303>Стоимость: " + std::to_string(m_cost) + "</color>"
            , {1.,0.});
    });
    m_button->onMouseLeave.disconnectAll();
    m_button->onMouseLeave.connect([&]() {
        GameState::Instance().set_tooltip_content("");
    });
}

void UpgradeButton::coins_update(int current_coins_count) {
    if (current_coins_count >= m_cost && m_state == State::Disabled)
        set_state(State::Active);
    if (current_coins_count < m_cost && m_state == State::Active)
        set_state(State::Disabled);
}

void UpgradeButton::achievement_event() {
    if (m_state != State::Locked) return;
    if (m_achievement_system_upgrade >= m_goal_upgrade_value) {
        set_state(State::Active);
        coins_update(GameState::Instance().get_player_coins());
    }
}


UpgradePanelCreator::UpgradePanelCreator() {
    panel = tgui::Group::create();
    panel->setSize("100%", "100%");
}

void UpgradePanelCreator::visit(MiniGun& minigun) {
    m_buttons.clear();
    panel->removeAllWidgets();

    m_buttons.resize(1);
    auto& shells_upgrades = m_buttons[0];
    //auto& cooling_upgrades = m_buttons[1];
    //auto& lubricant_upgrades = m_buttons[2];

    UpgradeButton shells_upgrade_I(TextureID::MinigunShellsUpgradeI, 1000, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 1, "Бронебойные снаряды I");
    //UpgradeButton shells_upgrade_II(TextureID::MinigunShellsUpgradeI, 1000, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 2);
    //UpgradeButton shells_upgrade_III(TextureID::MinigunShellsUpgradeI, 1000, minigun.m_penetration_upgrade, AchievementSystem::Instance().minigun_upgrades.penetration_upgrade, 3);
   
    shells_upgrades.push_back(std::move(shells_upgrade_I));
   // shells_upgrades.push_back(std::move(shells_upgrade_II));
    //shells_upgrades.push_back(std::move(shells_upgrade_III));

   /* cooling_upgrade.push_back(UpgradeButton(TextureID::MinigunCoolingUpgradeI, 1000, [&]() {
        minigun.cooling_upgrade();
    }));
    lubricant_upgrade.push_back(UpgradeButton(TextureID::MinigunLubricantUpgradeI, 1000, [&]() {
        minigun.lubricant_upgrade();
    }));*/

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
    panel->add(grid);
    
}

void UpgradePanelCreator::visit(Spikes& spikes) {

}

void UpgradePanelCreator::visit(Hedgehog& headgehogs) {

}

void UpgradePanelCreator::visit(AntitankGun& antitank_gun) {

}

void UpgradePanelCreator::visit(TwinGun& twingun) {

}

void UpgradePanelCreator::visit(Mine& mine) {

}

void UpgradePanelCreator::achievement_event() {
    for (auto& cat : m_buttons) {
        for (auto& up : cat) {
            up.achievement_event();
        }
    }
}

