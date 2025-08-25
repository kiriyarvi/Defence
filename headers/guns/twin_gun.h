#pragma once
#include "rotating_gun_base.h"
#include "utils/sprite_chain.h"
#include "params_manager.h"

class TwinGunAnimation {
public:
	TwinGunAnimation();
	void draw(sf::RenderWindow& window, int x_id, int y_id, float rotation);
	void draw_effects(sf::RenderWindow& window, int x_id, int y_id, float rotation);
	void logic(double dtime_microseconds, bool upper);
	void start_barrel_animation(bool upper);
	void fire_animation(sf::RenderWindow& window, int x_id, int y_id, float rotation, bool upper);
	void draw_enemy_hit_animation(sf::RenderWindow& window, int x_id, int y_id, bool upper);
public:
	double upper_barrel_offset = 0;
	double lower_barrel_offset = 0;
	double max_offset = 2;
	bool upper_barrel_animation = false;
	bool lower_barrel_animation = false;
	double upper_barrel_animation_dtime = 0;
	double lower_barrel_animation_dtime = 0;
	double duration = 1.5;
public:
	glm::vec2 upper_shot_fire_pos; // точка попадания по врагу, относительно центра врага.
	glm::vec2 lower_shot_fire_pos;
	uint32_t upper_barrel_shoted_enemy_id; // id врага, по которому попало верхнее орудие
	uint32_t lower_barrel_shoted_enemy_id; // id врага, по которому попало верхнее орудие
private:
	float compute_k(bool upper);
private:
	SpriteChain m_twin_gun_sprite;
	SpriteChain* m_upper_barrel;
	SpriteChain* m_lower_barrel;
};

class TwinGun : public IRotatingGun {
public:
	TwinGun();
	void draw(sf::RenderWindow& window, int x_id, int y_id) override;
	void draw_effects(sf::RenderWindow& window, int x, int y) override;
	void logic(double dtime_microseconds, int x_id, int y_id) override;
	void shoot_logic(int x_id, int y_id, IEnemy& enemy) override;
private:
	void shot(int x_id, int y_id, IEnemy& enemy, bool upper_barrel);
private:
	enum State {
		InterleavedCD,
		InterleavedReady,
		ShotCD,
		Ready
	} state = State::Ready;

	const ParamsManager::Params::Guns::TwinGun& m_params;

	double cd_shot = 0;
	double cd_interleaved = 0;
	TwinGunAnimation animation;
};
