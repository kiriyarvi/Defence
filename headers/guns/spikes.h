#pragma once
#include "guns/building_with_health.h"
#include "params_manager.h"


class Spikes : public BuildingWithHealth {
public:
	Spikes(int x_id, int y_id);
	void draw(sf::RenderWindow& window) override;
	void draw_effects(sf::RenderWindow& window) override {}
	void logic(double dtime) override;
	static sf::Sprite get_sprite_for_tile(int x_id, int y_id);
    ACCEPT(Spikes)
private:
	const ParamsManager::Params::Guns::Spikes& params;
	sf::Sprite sprite;
};


