#pragma once
#include "tile_map.h"
#include <functional>

class BuildingWithHealth : public IBuilding {
public:
    int get_health() { return m_health; }
    void set_health_changed_callback(const std::function<void()>& callback) { m_on_health_changed = callback; }
    void set_health(int health) { m_health = health; if (m_on_health_changed) m_on_health_changed(); }
    bool is_destroyed() override { return m_health <= 0; }
    bool auto_repair = false;
private:
    std::function<void()> m_on_health_changed;
    int m_health = 0;
};
