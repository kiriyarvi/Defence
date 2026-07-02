#pragma once
#include "tile_map.h"
#include "params_manager.h"

class RadioTower : public IBuilding {
public:
    RadioTower(int x_id, int y_id);
    void draw(sf::RenderWindow& window) override;
    void draw_effects(sf::RenderWindow& window) override;
    void logic(double dtime) override;
    ACCEPT(RadioTower)
    virtual ~RadioTower();
private:
    sf::Sprite m_tower_sprite;
};
