#pragma once
#include "enemies/IEnemy.h"
#include "utils/animation.h"

class Solder : public IEnemy {
public:
	Solder();
	void draw(sf::RenderWindow& window) override;
    void draw_effects(sf::RenderWindow& window) override;
	bool logic(double dtime) override;
	IDestroyedEnemy::Ptr get_destroyed_enemy() override;
private:
	sf::Sprite m_solder_sprite;
	sf::Sprite m_solder_ammunition;
	Animation m_animation;
	HealthIndicator m_health_indicator;
};
