#pragma once
#include "tile_map.h"
#include "params_manager.h"
#include "achievement_system.h"
#include "net_manager.h"
#include <list>


class Radar : public IBuilding {
    friend class NetManager::Net;
public:
    Radar(int x_id, int y_id);
    void draw(sf::RenderWindow& window) override;
    void draw_effects(sf::RenderWindow& window) override;
    void logic(double dtime) override;
    bool is_destroyed() override { return false; }
    ACCEPT(Radar)
    BuildingUpgrade radius_upgrade;
    BuildingUpgrade uncovering_level_upgrade;
    BuildingUpgrade uncovering_speed_upgrade;
    BuildingUpgrade long_distance_communication_upgrade;
    bool m_part_of_net = false;
private:
    sf::Sprite m_radar_sprite;
    sf::Sprite m_base_sprite;
    float m_rotation = 180;
    const ParamsManager::Params::Guns::Radar& m_params;

    struct Target {
        uint32_t target;
        float uncovering_time = 0;
    };
    float m_aiming_timer = 0.0;
    enum class Status {
        Aiming, // время наведения, не захватываем новые цели.
        Uncover
    } m_status = Status::Uncover;

    std::list<Target> m_targets;
};
