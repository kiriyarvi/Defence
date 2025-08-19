#include "antitank_gun.h"
#include "enemy_manager.h"
#include "sound_manager.h"

AntitankGun::AntitankGun() {
	radius = 3;
	m_turret_sprite.sprite.setTexture(TileMap::Instance().textures[TileTexture::AntitankGunTurret]);
	m_barrel = &m_turret_sprite.childs.emplace_back();
	m_barrel->sprite.setTexture(TileMap::Instance().textures[TileTexture::AntitankGunBarrel]);
	auto& substrate = m_turret_sprite.childs.emplace_back();
	substrate.sprite.setTexture(TileMap::Instance().textures[TileTexture::AntitankGunTurretSubstrate]);
	m_turret_sprite.layer = 2;
	m_barrel->layer = 1;
}

void AntitankGun::draw(sf::RenderWindow& window, int x_id, int y_id) {
	IRotatingGun::draw(window, x_id, y_id);

	m_turret_sprite.set_position_origin(9, 16);
	m_turret_sprite.set_position(x_id * 32 + 16, y_id * 32 + 16);
	m_turret_sprite.set_rotation_origin(9, 16);
	m_turret_sprite.set_rotation(rotation);
	m_barrel->set_position(m_barrel_offset, 0);
	if (m_animation)
		fire_animation();
	m_turret_sprite.draw(window);
}

void AntitankGun::fire_animation() {
	float k = m_animation_timer / (m_animation_duration * 1000 * 1000);
	float fraction = 0.10;
	if (k < fraction) {
		float p = k / fraction;
		int frame = p * 4;
		auto& fire = m_barrel->childs.begin();
		fire->sprite.setTextureRect(sf::IntRect(8 * frame,16, 8, 8));
	}
	else {
		if (!m_barrel->childs.empty())
			m_barrel->childs.clear();
	}

}

void AntitankGun::draw_effects(sf::RenderWindow& window, int x, int y) {
	if (!m_animation) return;
	Enemy* enemy = EnemyManager::Instance().get_enemy_by_id(m_shoted_enemy_id);
	if (!enemy) return;
	float k = m_animation_timer / (m_animation_duration * 1000 * 1000);
	float fraction = 0.15;
	if (k < fraction) {
		float p = k / fraction;
		int frame = p * 4;
		sf::Sprite fire(TileMap::Instance().textures[TileTexture::Shot]);
		fire.setTextureRect(sf::IntRect(8 * frame, 24, 8, 8));
		fire.setOrigin(4, 4);
		auto enemy_pos = enemy->get_position();
		fire.setPosition(enemy_pos.x, enemy_pos.y);
		window.draw(fire);
	}
}

void AntitankGun::logic(double dtime_microseconds, int x_id, int y_id) {
	if (m_state == State::CoolDown) {
		m_cd_time += dtime_microseconds;
		if (m_cd_time >= m_cooldown * 1000 * 1000)
			m_state = State::Ready;
	}
	if (m_animation) {
		m_animation_timer += dtime_microseconds;
		if (m_animation_timer >= m_animation_duration * 1000 * 1000)
			m_animation = false;
		else {
			float k = m_animation_timer / (m_animation_duration * 1000 * 1000);
			float fraction = 0.05;
			if (k <= fraction) {
				float p = k / fraction;
				m_barrel_offset = -p * 2.7;
			}
			else {
				float  p = (1. - k) / (1 - fraction);
				m_barrel_offset = -p * 2.7;
			}
		}
	}
	IRotatingGun::logic(dtime_microseconds, x_id, y_id);
}



void AntitankGun::shoot_logic(int x_id, int y_id, Enemy& enemy) {
	if (m_state == State::Ready) {
		enemy.health -= m_damage;
		SoundManager::Instance().play(Sounds::AntitankGunShot);
		m_state = State::CoolDown;
		m_cd_time = 0; // уходим на перезарядку.
		start_animation();
		m_shoted_enemy_id = enemy.id;
	}
}

void AntitankGun::start_animation() {
	m_animation = true;
	m_animation_timer = 0;
	auto& fire = m_barrel->childs.emplace_back();
	fire.sprite.setTexture(TileMap::Instance().textures[TileTexture::Shot]);
	fire.layer = 3;
	fire.set_position_origin(0, 4);
	fire.set_position(30, 16);
}