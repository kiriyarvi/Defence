#pragma once
#include "tile_map.h"
#include "params_manager.h"

class Spikes : public IBuilding {
public:
	Spikes();
	void draw(sf::RenderWindow& window, int x, int y) override;
	void draw_effects(sf::RenderWindow& window, int x, int y) override {}
	void logic(double dtime, int x, int y) override;
	bool is_destroyed() { return health <= 0; }
	static sf::Sprite get_sprite_for_tile(int x_id, int y_id);
    ACCEPT(Spikes)
private:
	const ParamsManager::Params::Guns::Spikes& params;
	int health;
	sf::Sprite sprite;
};


