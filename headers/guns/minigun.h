#pragma once
#include "guns/rotating_gun_base.h"
#include "utils/animation.h"
#include "utils/sprite_chain.h"
#include "params_manager.h"

class MiniGun : public IRotatingGun {
	friend class GameState;
public:
	MiniGun();
	void draw(sf::RenderWindow& window, int x_id, int y_id) override;
	void draw_effects(sf::RenderWindow& window, int x, int y) override;
	void logic(double dtime_microseconds, int x_id, int y_id) override;
	void shoot_logic(int x_id, int y_id, IEnemy& enemy) override;
	void on_gun_pointed() override;
	void on_gun_unpointed() override;
private:
	void temperature_logic(double dtime_microseconds);
	void drum_animation();
	void belt_animation();
	void compass_logic(int x_id, int y_id);
private:
	const ParamsManager::Params::Guns::Minigun& m_params;
	SpriteChain m_minigun_sprite;
	float m_heat_speed; // m_temperature в секунду
	float m_cooling_speed; // m_temperature в секунду
	float m_drum_angle = 0; // угол поворота барабана (нужно нормировать на 10^6).
	float m_temperature = 0; // max = 1'000'000
	float m_cooldown_timer = 0;
	float m_critical_temperature_mod_timer = 0;
	int m_penetration_upgrade = 0;

	enum class State {
		Heating,
		Cooling,
		Ready,
		CoolDown
	} m_state = State::Ready;

	enum class ShootState {
		CoolDown,
		Ready
	} m_shoot_state = ShootState::Ready;
	float m_shoot_timer = 0.0;

	bool m_dark_shell_on_belt = false;
	bool m_belt_supply = false;
	float m_belt_supply_timer = 0.0;
	float m_belt_position = 0;

	SpriteChain* m_belt_chain = nullptr;
	SpriteChain* m_enemy_compass = nullptr;

	Animation m_overheat_animation;
	ISpriteFramer::Ptr m_steam_framer;

	Animation m_shot_animation;
	ISpriteFramer::Ptr m_shot_framer;
	SpriteChain* m_shot_sprite = nullptr;

	uint32_t m_shoted_enemy_id;
	Animation m_enemy_hit_animation;
	ISpriteFramer::Ptr m_enemy_hit_framer;
	glm::vec2 m_random_hit_offset = glm::vec2(0,0);

	Animation m_enemy_rebound_animation;
	ISpriteFramer::Ptr m_enemy_rebound_framer;
};