#pragma once
#include "guns/rotating_gun_base.h"
#include "utils/animation.h"
#include "utils/sprite_chain.h"


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
	SpriteChain m_minigun_sprite;
	float m_heat_speed = 1 / 12.; // m_temperature в секунду
	float m_cooling_speed = 1 / 7.; // m_temperature в секунду
	float m_min_rotation_speed = 10; // градусов в секунду
	float m_max_rotation_speed = 2*360;
	float m_drum_angle = 0; // угол поворота барабана (нужно нормировать на 10^6).
	float m_min_damage = 1;
	float m_max_damage = 1;
	float m_temperature = 0; // max = 1'000'000

	float m_cooldown_duration = 7; // время прекращения работы после перегрева.
	float m_cooldown_timer = 0;
	float m_critical_temperature = 0.7;
	float m_critical_temperature_work_duration = 4; // максимальное время работы при критическом нагреве.
	float m_critical_temperature_mod_timer = 0;

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
};