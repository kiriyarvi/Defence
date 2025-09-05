#pragma once
#include "building_with_health.h"
#include "params_manager.h"

class Hedgehog : public BuildingWithHealth {
public:
	Hedgehog();
	void draw(sf::RenderWindow& window, int x, int y) override;
	void draw_effects(sf::RenderWindow& window, int x, int y) override {}
	void logic(double dtime, int x, int y) override;
    ACCEPT(Hedgehog)
private:
	const ParamsManager::Params::Guns::Hedgehog& params;
	sf::Sprite sprite;
};
