#pragma once
#include <string>
#include <list>
#include "gui/label.h"
#include "guns/building.h"
#include "achievement_system.h"
#include "utils/animation.h"


class Console : public Widget {
public:
    void add_message(const std::string& string);
    void add_message(std::unique_ptr<Label>&& label);
    void add_building_unlock_message(BuildingType type);
    void add_upgrade_unlock_message(Upgrade& upgrade, int level);
    void logic(float dtime_mc);
private:
    void update_rules();
    struct Message {
        Widget* widget;
        Animation animation;
    };
    std::list<Message> m_messages;
};
