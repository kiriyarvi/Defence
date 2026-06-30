#pragma once
#include "tile_map.h"
#include "params_manager.h"

class RadioTower : public IBuilding {
public:
    RadioTower();
    void draw(sf::RenderWindow& window, int x, int y) override;
    void draw_effects(sf::RenderWindow& window, int x, int y) override;
    void logic(double dtime, int x_id, int y_id) override;
    ACCEPT(RadioTower)
private:
    sf::Sprite m_tower_sprite;
};
