#pragma once
#include "tile_map.h"
#include "utils/animation.h"

class Mine: public IBuilding {
public:
	Mine();
	void draw(sf::RenderWindow& window, int x, int y) override;
	void draw_effects(sf::RenderWindow& window, int x, int y) override;
	void logic(double dtime, int x_id, int y_id) override;
	bool is_destroyed() override { return m_state == State::Destroyed; }
private:
	enum class State {
		Ready,
		Activated,
		Destroyed
	} m_state = State::Ready;
	sf::Sprite m_mine_sprite;

	float m_damage_radius = 2;
	float m_min_damage = 1;
	float m_max_damage = 40;

	Animation m_blast_animation;
	ISpriteFramer::Ptr m_blast_framer = nullptr;
};


