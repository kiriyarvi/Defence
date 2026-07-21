#pragma once
#include "gui/tiled_panel.h"
#include "gui/icon_button.h"
#include "gui/icon.h"
#include "guns/building_with_health.h"

#include "tile_map.h"

#include <functional>

class UpgradePanel;
class Upgrade;

class UpgradeButton : public LayeredIcon {
public:
    UpgradeButton(
        Widget* ui,
        UpgradePanel* upgrade_panel,
        UpgradeButton* prev_upgrade_button,
        IBuilding* building,
        Upgrade* upgrade,
        int level,
        TextureID upgrade_icon
    );

    enum class State {
        ACTIVE,
        BOUGTH,
        NOT_ENOGTH_MONEY,
        UNDISCOVERED,
    };
    void set_state(State state);
    State get_state() { return m_state; }
    void set_capture(bool capture);
    void update(int player_coins);
    Query on_event(EventContext event_context);
    Upgrade* get_upgrate() const { return m_upgrade; }
    IBuilding* get_building() const { return m_building; }
    int get_level() const { return m_level; }
    bool is_captured() const { return m_captured; };
private:
    void create_tooltip_smart();
    void delete_tooltip_smart();
    UpgradeButton* m_prev_upgrade_button;
    IBuilding* m_building;
    UpgradePanel* m_upgrade_panel;
    bool m_captured = false;
    State m_state = State::UNDISCOVERED;
    TextureID m_upgrade_icon;
    Icon* m_capture_icon = nullptr;
    Upgrade* m_upgrade;
    int m_level;
    Widget* m_tooltip = nullptr;
    Widget* m_ui;
    bool m_clicked = false;
};

class UpgradePanel : public TiledPanel, public IBuildingVisitor {
public:
    UpgradePanel(Widget* tile_size_reference, Widget* height_reference);
    void visit(MiniGun& minigun) override;
    void visit(Spikes& spikes) override;
    void visit(Hedgehog& headgehogs) override;
    void visit(AntitankGun& antitank_gun) override;
    void visit(TwinGun& twingun) override;
    void visit(Mine& mine) override;
    void visit(Radar& radar) override;
    void visit(RadioMast& radio_tower) override;
    void capture(UpgradeButton* button);
    void update(int player_coins);
    Query on_event(EventContext event_context) override;
    ~UpgradePanel();
private:
    Widget* create_buttons_for_upgrade(Widget* parent, IBuilding* building, Upgrade* upgrade, const std::vector<TextureID>& upgrade_buttons_icons);
    void create_info_panel_for_button(UpgradeButton* button);
    void create_panel_for_building_with_health(BuildingType type, BuildingWithHealth* building, int enforce_cost, int repairing_hp);
    Widget* m_tile_size_reference;
    Widget* m_height_reference;
    Widget* m_upgrade_info_widget; 
    std::vector<UpgradeButton*> m_upgrade_buttons;
    std::vector<std::function<void()>> m_on_kill_actions;
    std::vector<std::function<void(int)>> m_update_callbacks;
};


//
//
//class UpgradeButton: public IconButton {
//public:
//	UpgradeButton(TextureID icon, int cost, UpgradeableProperty& building_upgrade,  int& achievement_system_upgrade, int goal_upgrade_value, const std::string name);
//	UpgradeButton(UpgradeButton&& btn);
//	UpgradeButton(const UpgradeButton&) = delete;
//	UpgradeButton& operator=(const UpgradeButton&) = delete;
//    void update();
//	std::function<void()> on_mouse_enter;
//	std::function<void()> on_mouse_leave;
//private:
//	void connect();
//	int m_cost;
//	UpgradeableProperty& m_building_upgrade;
//	int& m_achievement_system_upgrade;
//	int m_goal_upgrade_value;
//	std::string m_name;
//    std::string m_reason;
//};
//
//
//class  UpgradePanelCreator : public IBuildingVisitor {
//public:
//	UpgradePanelCreator();
//	void update();
//	virtual void visit(MiniGun& minigun);
//	virtual void visit(Spikes& spikes);
//	virtual void visit(Hedgehog& headgehogs);
//	virtual void visit(AntitankGun& antitank_gun);
//	virtual void visit(TwinGun& twingun);
//	virtual void visit(Mine& mine);
//  virtual void visit(Radar& radar);
//  virtual void visit(RadioMast& radio_tower);
//	tgui::Group::Ptr panel; //основная панель контента. Заполняется в visit
//    tgui::Group::Ptr info;
//    ~UpgradePanelCreator();
//    void reset(); //internal use only. Очищает panel панель.
//    std::vector<std::function<void()>> m_clear_actions; //internal use only
//private:
//    tgui::Button::Ptr create_sell_button(int x_id, int y_id);
//    std::function<int()> m_compute_cost;
//	std::vector<std::vector<UpgradeButton>> m_buttons;
//};

