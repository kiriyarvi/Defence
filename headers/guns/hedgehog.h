#pragma once
#include "building_with_health.h"
#include "params_manager.h"

class Hedgehog : public BuildingWithHealth {
public:
	Hedgehog(int x_id, int y_id);
	void draw(sf::RenderWindow& window) override;
	void draw_effects(sf::RenderWindow& window) override {}
	void logic(double dtime) override;
    ACCEPT(Hedgehog)
private:
	const ParamsManager::Params::Guns::Hedgehog& params;
	sf::Sprite sprite;
};
