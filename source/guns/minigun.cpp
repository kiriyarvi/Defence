#include "guns/minigun.h"
#include "enemy_manager.h"
#include "sound_manager.h"

#include "game_state.h"
#include "utils/framers.h"

#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/random.hpp"

MiniGun::MiniGun() {
	radius = 2;

	m_minigun_sprite.sprite.setTexture(TileMap::Instance().textures[TileTexture::MiniGun]);
	m_minigun_sprite.set_position_origin(16, 16);
	m_minigun_sprite.set_rotation_origin(16, 16);
	m_minigun_sprite.layer = -1;

	sf::Sprite drum(TileMap::Instance().textures[TileTexture::MiniGunEquipment]);
	drum.setOrigin(0, 1.5);
	drum.setScale(4 / 9.f, 4 / 9.f);
	drum.setTextureRect(sf::IntRect(0, 24, 16, 3));
	for (int i = 0; i < 6; ++i)
		m_minigun_sprite.childs.push_back(SpriteChain(drum));

	sf::Sprite belt(TileMap::Instance().textures[TileTexture::MiniGunEquipment]);
	belt.setTextureRect(sf::IntRect(0, 0, 10, 24));
	belt.setScale(3 / 10., 8. / 24.);
	m_belt_chain = &m_minigun_sprite.childs.emplace_back(SpriteChain(belt));
	m_belt_chain->layer = 1;

	sf::Sprite overlap(TileMap::Instance().textures[TileTexture::MiniGunEquipment]);
	overlap.setTextureRect(sf::IntRect(10, 0, 3, 10));
	auto& overlap_chain = m_minigun_sprite.childs.emplace_back(SpriteChain(overlap));
	overlap_chain.layer = 2;
	overlap_chain.set_position(10, 10);

	m_steam_framer = std::make_unique<SteamFramer>();
	m_overheat_animation.set_duration(0.5);
	m_overheat_animation.add_framer(m_steam_framer);
	m_overheat_animation.set_loop(true);
	m_overheat_animation.start();

	sf::Sprite compass(TileMap::Instance().textures[TileTexture::MiniGunEquipment]);
	compass.setTextureRect(sf::IntRect(16, 0, 5, 5));
	compass.scale(3. / 5., 3. / 5.);
	m_enemy_compass = &m_minigun_sprite.childs.emplace_back(SpriteChain(compass));
	m_enemy_compass->layer = 0;
	m_enemy_compass->set_position(14,12);

	float min_cooldown_time = (360 / m_max_rotation_speed) / 6;
	float shot_offset_time = min_cooldown_time * 0.5;
	m_shot_animation.set_duration(shot_offset_time);
	m_shot_framer = std::make_unique<MinigunShootFramer>();
	m_shot_animation.add_framer(m_shot_framer);
	m_shot_sprite = &m_minigun_sprite.childs.emplace_back();
	m_shot_sprite->set_position(22 + 4 / 9.f * 16, 18);
	m_shot_sprite->layer = 10;
	m_shot_sprite->enabled = false;

	m_enemy_hit_framer = std::make_unique<MinigunHitFramer>();
	m_enemy_hit_animation.set_duration(shot_offset_time);
	m_enemy_hit_animation.add_framer(m_enemy_hit_framer);
}

void MiniGun::draw(sf::RenderWindow& window, int x_id, int y_id) {
	IRotatingGun::draw(window, x_id, y_id);
	m_minigun_sprite.set_position(32 * x_id + 16, 32 * y_id + 16);
	m_minigun_sprite.set_rotation(rotation);
	if (m_shoot_state == ShootState::CoolDown && m_shot_animation.started()) {
		m_shot_sprite->sprite = m_shot_framer->sprite;
		//m_shot_sprite->sprite.setScale(4 / 9.f, 4 / 9.f);
	}
	m_minigun_sprite.draw(window);
	if (m_state == State::CoolDown) {
		auto sprite = m_steam_framer->sprite;
		for (int i = 0; i < 3; ++i) {
			glm::vec2 pos = glm::vec2(x_id * 32 + 16, y_id * 32 + 16) + glm::rotate<float>(glm::vec2(6 + 3 * i, 2), glm::radians(rotation));
			sprite.setPosition(pos.x, pos.y);
			window.draw(sprite);
		}
	}
}

void MiniGun::draw_effects(sf::RenderWindow& window, int x, int y) {
	if (!m_enemy_hit_animation.started()) return;
	IEnemy* enemy = EnemyManager::Instance().get_enemy_by_id(m_shoted_enemy_id);
	if (!enemy) return;
	auto enemy_pos = enemy->get_position() + m_random_hit_offset;
	m_enemy_hit_framer->sprite.setPosition(enemy_pos.x, enemy_pos.y);
	window.draw(m_enemy_hit_framer->sprite);
}

void MiniGun::on_gun_pointed() {
	if (m_state != State::CoolDown)
		m_state = State::Heating;
}

void MiniGun::on_gun_unpointed() {
	if (m_state != State::CoolDown)
		m_state = State::Cooling;
}

void MiniGun::drum_animation() {
	auto& barrels = m_minigun_sprite.childs;
	auto it = barrels.begin();
	for (int i = 0; i < 6; ++i, ++it) {
		float angle = glm::radians(m_drum_angle / (1000 * 1000) + 60 * i);
		it->set_position(22, 18 + (4/3.) * cos(angle));
		it->layer = sin(angle) * 6;
		float darkness = (1. + sin(angle)) / 2.f;
		darkness = 0.3 + darkness * 0.7;
		float heat = 1.f - (std::min(m_temperature, m_critical_temperature * 1000 * 1000)) / (1000 * 1000);
		it->sprite.setColor(sf::Color(255.f * darkness, 255.f * darkness * heat, 255.f * darkness * heat));
	}
}

void MiniGun::belt_animation() {
	float frac = m_belt_position;
	frac = frac - int(frac);
	m_belt_chain->set_position(10, 10 + 2 * frac);
}

void MiniGun::logic(double dtime_microseconds, int x_id, int y_id) {
	temperature_logic(dtime_microseconds);

	float t = m_temperature / (1000 * 1000);
	float rotation_speed = m_min_rotation_speed + t * (m_max_rotation_speed - m_min_rotation_speed); //вращение, градусов в секунду

	if (m_shoot_state == ShootState::CoolDown) {
		float cooldown_time = (360 / rotation_speed) / 6;// время полного оборота / 6;
		m_shoot_timer += dtime_microseconds;
		if (m_shoot_timer >= cooldown_time * 1000 * 1000) {
			m_shoot_state = ShootState::Ready;
			m_shot_sprite->enabled = false;
		}
		m_shot_animation.logic(dtime_microseconds);
		if (!m_shot_animation.started()) {
			m_shot_sprite->enabled = false;
		}
		m_enemy_hit_animation.logic(dtime_microseconds);
	}

	if (m_state == State::Heating)
		m_drum_angle += rotation_speed * dtime_microseconds;
	drum_animation();
	if (m_belt_supply) {
		float min_cooldown_time = (360 / m_max_rotation_speed) / 6;
		float shot_offset_time = min_cooldown_time * 0.5;
		m_belt_supply_timer += dtime_microseconds;
		float p = (m_belt_supply_timer / (shot_offset_time * 1000 * 1000));

		m_belt_position = p * 0.5 + (m_dark_shell_on_belt ? 0.5 : 0);

		if (p >= 1) {
			m_belt_supply = false;
			m_dark_shell_on_belt = !m_dark_shell_on_belt;
		}
	}
	belt_animation();
	IRotatingGun::logic(dtime_microseconds, x_id, y_id);
	compass_logic(x_id, y_id);
	GameState::Instance().minigun_state_update(*this);
}

void MiniGun::temperature_logic(double dtime_microseconds) {
	if (m_state == State::Heating) {
		m_temperature += dtime_microseconds * m_heat_speed;
		m_temperature = std::min(m_temperature, 1000 * 1000.f);
		if (m_temperature > 1000 * 1000 * m_critical_temperature) {
			m_critical_temperature_mod_timer += dtime_microseconds;
		}
	}
	else if (m_state == State::Cooling) {
		m_temperature -= dtime_microseconds * m_cooling_speed;
		if (m_temperature > 1000 * 1000 * m_critical_temperature) {
			m_critical_temperature_mod_timer += dtime_microseconds;
		}
		if (m_temperature < 0) {
			m_state = State::Ready;
		}
	}
	else if (m_state == State::CoolDown) {
		m_cooldown_timer += dtime_microseconds;
		m_temperature -= dtime_microseconds * m_cooling_speed;
		m_temperature = std::max(0.f, m_temperature);
		if (m_cooldown_timer >= m_cooldown_duration * 1000 * 1000) {
			if (!m_is_enemy_captured)
				m_state = State::Ready;
			else {
				if (m_is_gun_pointed)
					m_state = State::Heating;
				else
					m_state = State::Cooling;
			}
		}
		m_overheat_animation.logic(dtime_microseconds);
	}

	if (m_critical_temperature_mod_timer >= m_critical_temperature_work_duration * 1000 * 1000) {
		m_state = State::CoolDown;
		m_cooldown_timer = 0;
		SoundManager::Instance().play(Sounds::OverHeat);
		m_shoot_state = ShootState::Ready;
		m_shot_sprite->enabled = false;
		m_critical_temperature_mod_timer = 0;
	}
	
}

void MiniGun::shoot_logic(int x_id, int y_id, IEnemy& enemy) {
	if (m_shoot_state == ShootState::Ready && m_state != State::CoolDown) {
		enemy.health -= int((m_temperature / (1000 * 1000.f)) * (m_max_damage - m_min_damage) + m_min_damage);
		m_shoot_state = ShootState::CoolDown;
		m_shoot_timer = 0;
		SoundManager::Instance().play(Sounds::MiniGunShot,10, 25 + (rand() % 100) / 100.f * 7, 1.0 - (rand() % 100) / 100.f * 0.05);
		if (m_state == State::Ready)
			m_state = State::Heating;
		m_belt_supply = true;
		m_belt_supply_timer = 0.0;

		m_shot_sprite->enabled = true;
		m_shot_animation.start();
		m_shot_animation.logic(0.0); // чтобы выставить первый фрейм.

		m_shoted_enemy_id = enemy.id;

		m_enemy_hit_animation.start();
		m_enemy_hit_animation.logic(0.0); // чтобы выставить первый фрейм.
		m_random_hit_offset = glm::circularRand<float>(2);
	}
}

void MiniGun::compass_logic(int x_id, int y_id) {
	if (m_is_enemy_captured) {
		glm::vec2 pos(x_id * 32 + 16, y_id * 32 + 16);
		auto enemy = EnemyManager::Instance().get_enemy_by_id(m_captured_enemy_id); // по идее можно не беспокоиться о том, что enemy = nullptr, потому что эта функция вызывается после IRotatingBase::logic()
		float rotation_rad = glm::radians(rotation);
		glm::mat2x2 A{
			{cos(rotation_rad), sin(rotation_rad)},
			{-sin(rotation_rad), cos(rotation_rad)}
		};
		glm::vec2 dir = enemy->get_position() - pos;
		dir.y = -dir.y; // из-за того что базис изначально левый. Нужно перевести в правый, чтобы корректно отработал atan.
		dir = A * dir;
		float angle = glm::atan(dir.y, dir.x); //[-pi, pi]
		if (angle < 0)
			angle += glm::two_pi<float>();
		int frame = ((angle / glm::two_pi<float>()) * 8);
		if (frame == 8)
			frame = 0;
		m_enemy_compass->sprite.setTextureRect(sf::IntRect(16 + 5 * (frame % 3), 5 * (frame / 3), 5, 5));
	}
}

