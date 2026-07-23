#pragma once
#include "guns/building.h"
#include "params_manager.h"
#include "net_manager.h"
#include "enemies/enemy_container.h"
#include <list>


class Radar : public IBuilding {
    friend class NetManager::Net;
public:
    Radar(int x_id, int y_id);
    void draw(sf::RenderWindow& window) override;
    void draw_effects(sf::RenderWindow& window) override;
    void logic(double dtime) override;
    bool is_destroyed() override { return false; }
    ACCEPT(Radar);
    void upgrade_radius(int level);
    void upgrade_uncovering_level(int level);
    void upgrade_uncovering_speed(int level);
    void upgrade_long_distance_communication();
    int radius_upgrade = 0;
    int uncovering_level_upgrade = 0;
    int uncovering_speed_upgrade = 0;
    int long_distance_communication_upgrade = 0;
    bool m_part_of_net = false;
    virtual ~Radar();
private:
    sf::Sprite m_radar_sprite;
    sf::Sprite m_base_sprite;
    float m_rotation = 180;
    const ParamsManager::Params::Guns::Radar& m_params;

    struct Target {
        EnemyContainer::EnemyID target;
        float uncovering_time = 0;
    };
    float m_aiming_timer = 0.0;
    enum class Status {
        Aiming, // время наведения, не захватываем новые цели.
        Uncover
    } m_status = Status::Uncover;

    std::list<Target> m_targets;
};
