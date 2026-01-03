#pragma once
#include "tile_map.h"
#include "params_manager.h"

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
};
