#pragma once
#include "guns/building.h"
#include "params_manager.h"

class RadioMast : public IBuilding {
public:
    RadioMast(int x_id, int y_id);
    void draw(sf::RenderWindow& window) override;
    void draw_effects(sf::RenderWindow& window) override;
    void logic(double dtime) override;
    ACCEPT(RadioMast)
    virtual ~RadioMast();
private:
    sf::Sprite m_tower_sprite;
};
