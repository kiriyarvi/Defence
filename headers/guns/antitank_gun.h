#pragma once
#include "rotating_gun_base.h"
#include "utils/sprite_chain.h"
#include "enemies/enemy_container.h"
#include "params_manager.h"

class AntitankGun: public IRotatingGun {
public:
	AntitankGun(int x_id, int y_id);
	void draw(sf::RenderWindow& window) override;
	void draw_effects(sf::RenderWindow& window) override;
	void logic(double dtime_microseconds) override;
	void shoot_logic(IEnemy& enemy) override;
    ACCEPT(AntitankGun)
private:
	enum class State {
		Ready,
		CoolDown
	} m_state = State::Ready;
	void start_animation();
	void fire_animation();
protected:
    TargetStatus get_enemy_status(IEnemy& enemy) override;
private:
	const ParamsManager::Params::Guns::Antitank& m_params;

	double m_cd_time = 0;

	SpriteChain m_turret_sprite;
	SpriteChain* m_barrel;

	bool m_animation = false;
	double m_animation_duration = 1.5;
	double m_animation_timer = 0.0;
	double m_barrel_offset = 0.0;
	EnemyContainer::EnemyID m_shoted_enemy_id;
};
