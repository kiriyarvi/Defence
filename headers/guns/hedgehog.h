#pragma once
#include "tile_map.h"
#include "params_manager.h"

class Hedgehog : public IBuilding {
public:
	Hedgehog();
	void draw(sf::RenderWindow& window, int x, int y) override;
	void draw_effects(sf::RenderWindow& window, int x, int y) override {}
	void logic(double dtime, int x, int y) override;
	bool is_destroyed() { return health <= 0; }
    ACCEPT(Hedgehog)
private:
	const ParamsManager::Params::Guns::Hedgehog& params;
	int health;
	sf::Sprite sprite;
};
