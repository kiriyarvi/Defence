#pragma once
#include "tile_map.h"
#include "params_manager.h"
#include <list>

class Radar : public IBuilding {
public:
    Radar();
    void draw(sf::RenderWindow& window, int x, int y) override;
    void draw_effects(sf::RenderWindow& window, int x, int y) override;
    void logic(double dtime, int x_id, int y_id) override;
    bool is_destroyed() override { return false; }
    ACCEPT(Radar)
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
