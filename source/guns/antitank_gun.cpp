#include "guns/antitank_gun.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "texture_manager.h"

AntitankGun::AntitankGun(): m_params(ParamsManager::Instance().params.guns.antitank) {
	radius = m_params.radius;

	m_turret_sprite.sprite.setTexture(TextureManager::Instance().textures[TextureID::AntitankGunTurret]);
	m_barrel = &m_turret_sprite.childs.emplace_back();
	m_barrel->sprite.setTexture(TextureManager::Instance().textures[TextureID::AntitankGunBarrel]);
	auto& substrate = m_turret_sprite.childs.emplace_back();
	substrate.sprite.setTexture(TextureManager::Instance().textures[TextureID::AntitankGunTurretSubstrate]);
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
	IEnemy* enemy = EnemyManager::Instance().get_enemy_by_id(m_shoted_enemy_id);
	if (!enemy) return;
	float k = m_animation_timer / (m_animation_duration * 1000 * 1000);
	float fraction = 0.15;
	if (k < fraction) {
		float p = k / fraction;
		int frame = p * 4;
		sf::Sprite fire(TextureManager::Instance().textures[TextureID::Shot]);
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
		if (m_cd_time >= m_params.cooldown * 1000 * 1000)
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



void AntitankGun::shoot_logic(int x_id, int y_id, IEnemy& enemy) {
	if (m_state == State::Ready) {
        if (enemy.params.armor_level <= m_params.armor_penetration_level) {
            enemy.health -= m_params.damage;
            SoundManager::Instance().play(Sounds::AntitankGunShot);
        }
        else {
            SoundManager::Instance().play(Sounds::Ricochet);
        }
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
	fire.sprite.setTexture(TextureManager::Instance().textures[TextureID::Shot]);
	fire.layer = 3;
	fire.set_position_origin(0, 4);
	fire.set_position(30, 16);
}

IRotatingGun::TargetStatus AntitankGun::get_enemy_status(IEnemy& enemy) {
    TargetStatus status;
    // не атакуем слишком бронированных врагов и пехоту
    if (enemy.params.armor_level > m_params.armor_penetration_level || (enemy.infantry && enemy.wheels == IEnemy::Wheels::None)) {
        status.valid = false;
        return status;
    }
    status.mult_by_distance = true;
    return status;
}


