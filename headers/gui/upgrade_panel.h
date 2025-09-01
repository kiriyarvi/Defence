#pragma once
#include "gui/icon_button.h"
#include <functional>
#include "guns/minigun.h"
#include "guns/spikes.h"
#include "guns/hedgehog.h"
#include "guns/antitank_gun.h"
#include "guns/twin_gun.h"
#include "guns/mine.h"

class UpgradeButton: public IconButton {
public:
	UpgradeButton(TextureID icon, int cost, BuildingUpgrade& building_upgrade,  int& achievement_system_upgrade, int goal_upgrade_value, const std::string name);
	UpgradeButton(UpgradeButton&& btn);
	UpgradeButton(const UpgradeButton&) = delete;
	UpgradeButton& operator=(const UpgradeButton&) = delete;
    void update();
	std::function<void()> on_mouse_enter;
	std::function<void()> on_mouse_leave;
private:
	void connect();
	int m_cost;
	BuildingUpgrade& m_building_upgrade;
	int& m_achievement_system_upgrade;
	int m_goal_upgrade_value;
	std::string m_name;
    std::string m_reason;
};


class  UpgradePanelCreator : public IBuildingVisitor {
public:
	UpgradePanelCreator();
	void update();
	virtual void visit(MiniGun& minigun);
	virtual void visit(Spikes& spikes);
	virtual void visit(Hedgehog& headgehogs);
	virtual void visit(AntitankGun& antitank_gun);
	virtual void visit(TwinGun& twingun);
	virtual void visit(Mine& mine);
	tgui::Group::Ptr panel;
    tgui::Group::Ptr info;
private:
	std::vector<std::vector<UpgradeButton>> m_buttons;
};






