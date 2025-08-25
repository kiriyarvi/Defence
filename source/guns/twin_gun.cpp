#include "guns/twin_gun.h"
#include "sound_manager.h"
#include "enemy_manager.h"



TwinGunAnimation::TwinGunAnimation() {
	m_twin_gun_sprite.sprite.setTexture(TileMap::Instance().textures[TileTexture::TwinGunTurret]);
	m_twin_gun_sprite.layer = 1;
	SpriteChain upper_barrel;
	upper_barrel.sprite.setTexture(TileMap::Instance().textures[TileTexture::TwinGunUpperBarrel]);
	upper_barrel.layer = 0;
	SpriteChain lower_barrel;
	lower_barrel.sprite.setTexture(TileMap::Instance().textures[TileTexture::TwinGunUpperBarrel]);
	lower_barrel.layer = 0;
	m_twin_gun_sprite.childs.push_back(upper_barrel);
	m_twin_gun_sprite.childs.push_back(lower_barrel);
	m_upper_barrel = &*m_twin_gun_sprite.childs.begin();
	m_lower_barrel = &*(++m_twin_gun_sprite.childs.begin());
}

void TwinGunAnimation::draw(sf::RenderWindow& window, int x_id, int y_id, float rotation) {
	m_twin_gun_sprite.set_position_origin(16, 15.5);
	m_twin_gun_sprite.set_position(x_id * 32 + 16, y_id * 32 + 16);
	m_twin_gun_sprite.set_rotation_origin(16, 15.5);
	m_twin_gun_sprite.set_rotation(rotation);

	m_upper_barrel->set_position(-upper_barrel_offset, 0);
	m_lower_barrel->set_position(-lower_barrel_offset, 5);

	fire_animation(window, x_id, y_id, rotation, true);
	fire_animation(window, x_id, y_id, rotation, false);

	m_twin_gun_sprite.draw(window);
}

void TwinGunAnimation::draw_effects(sf::RenderWindow& window, int x_id, int y_id, float rotation) {
	draw_enemy_hit_animation(window, x_id, y_id, true);
	draw_enemy_hit_animation(window, x_id, y_id, false);
}

float TwinGunAnimation::compute_k(bool upper) {
	return (upper ? upper_barrel_animation_dtime : lower_barrel_animation_dtime) / (duration * 1000 * 1000);
}

void TwinGunAnimation::fire_animation(sf::RenderWindow& window, int x_id, int y_id, float rotation, bool upper) {
	if (!(upper ? upper_barrel_animation : lower_barrel_animation)) return;
	SpriteChain* barrel = upper ? m_upper_barrel : m_lower_barrel;
	float k = compute_k(upper);
	double fire_duration = 0.1;
	if (k > fire_duration) {
		barrel->childs.clear();
		return;
	}
	k = k / fire_duration;
	//TODO вообще нужно менять спрайт только один раз. Так один спрайт кучу раз устанавливается каждый кадр.
	sf::Sprite fire(TileMap::Instance().textures[TileTexture::Shot]);
	if (k < 1 / 3.)
		fire.setTextureRect(sf::IntRect(0, 0, 8, 8));
	else if (k < 2. / 3.)
		fire.setTextureRect(sf::IntRect(8, 0, 8, 8));
	else
		fire.setTextureRect(sf::IntRect(16, 0, 8, 8));
	barrel->childs.begin()->sprite = fire;
}

void TwinGunAnimation::draw_enemy_hit_animation(sf::RenderWindow& window, int x_id, int y_id, bool upper) {
	if (!(upper ? upper_barrel_animation : lower_barrel_animation)) return;

	float k = compute_k(upper);
	double fire_duration = 0.15;
	if (k > fire_duration)
		return;
	k = k / fire_duration;

	uint32_t& enemy_id = upper ? upper_barrel_shoted_enemy_id : lower_barrel_shoted_enemy_id;
	IEnemy* enemy = EnemyManager::Instance().get_enemy_by_id(enemy_id);
	if (!enemy) return;
	glm::vec2 enemy_pos = enemy->get_position();
	glm::vec2 hit_pos = enemy_pos + (upper ? upper_shot_fire_pos : lower_shot_fire_pos);

	sf::Sprite fire(TileMap::Instance().textures[TileTexture::Shot]);
	if (k < 1 / 3.)
		fire.setTextureRect(sf::IntRect(0, 8, 8, 8));
	else if (k < 2. / 3.)
		fire.setTextureRect(sf::IntRect(8, 8, 8, 8));
	else
		fire.setTextureRect(sf::IntRect(16, 8, 8, 8));
	fire.setPosition(hit_pos.x, hit_pos.y);
	fire.setOrigin(4, 4);
	window.draw(fire);
}

void TwinGunAnimation::logic(double dtime_microseconds, bool upper) {
	bool& is_animation = upper ? upper_barrel_animation : lower_barrel_animation;
	double& animation_dtime = (upper ? upper_barrel_animation_dtime : lower_barrel_animation_dtime);
	double& offset = (upper ? upper_barrel_offset : lower_barrel_offset);
	SpriteChain* barrel = upper ? m_upper_barrel : m_lower_barrel;
	if (is_animation) {
		animation_dtime += dtime_microseconds;
		double k = animation_dtime / (duration * 1000 * 1000);
		double p = 0.05;
		if (k < p) {
			offset = max_offset * k / p;
		}
		else {
			offset = max_offset * ((1 - (k - p) / (1 - p)));
		}
		if (k > 1.) {
			is_animation = false;
			offset = 0.0;
			animation_dtime = 0;
			barrel->childs.clear();
		}
	}
}

void TwinGunAnimation::start_barrel_animation(bool upper) {
	bool& is_animation = upper ? upper_barrel_animation : lower_barrel_animation;
	SpriteChain* barrel = upper ? m_upper_barrel : m_lower_barrel;
	is_animation = true;
	auto& fire = barrel->childs.emplace_back();
	fire.set_position(29, 9);
}


TwinGun::TwinGun(): m_params(ParamsManager::Instance().params.guns.twingun) {
	radius = m_params.radius;
}

void TwinGun::draw(sf::RenderWindow& window, int x_id, int y_id) {
	IRotatingGun::draw(window, x_id, y_id);
	animation.draw(window, x_id, y_id, rotation);
}

void TwinGun::draw_effects(sf::RenderWindow& window, int x, int y) {
	animation.draw_effects(window, x, y,rotation);
}

void TwinGun::logic(double dtime_microseconds, int x_id, int y_id) {
	animation.logic(dtime_microseconds, true);
	animation.logic(dtime_microseconds, false);
	if (state == State::ShotCD) {
		cd_shot += dtime_microseconds;
		if (cd_shot >= m_params.cooldown * 1000 * 1000)
			state = State::Ready;
	}
	else if (state == State::InterleavedCD) {
		cd_interleaved += dtime_microseconds;
		if (cd_interleaved >= m_params.interleaved_cooldown * 1000 * 1000)
			state = State::InterleavedReady;
	}
	IRotatingGun::logic(dtime_microseconds, x_id, y_id);
}

void TwinGun::shot(int x_id, int y_id, IEnemy& enemy, bool upper_barrel) {
	enemy.health -= m_params.damage_per_barrel;
	SoundManager::Instance().play(Sounds::TwinGunShot);
	glm::vec3 gun_pos(x_id * 32 + 16, y_id * 32 + 16, 0);
	glm::vec3 enemy_pos = glm::vec3(enemy.get_position(), 0.0);
	glm::vec3 up(0, 0, 1);
	glm::vec3 dir = enemy_pos - gun_pos;
	glm::vec3 perp = 3.f * glm::normalize(glm::cross(dir, up));
	uint32_t& save_enemy_id = upper_barrel ? animation.upper_barrel_shoted_enemy_id : animation.lower_barrel_shoted_enemy_id;
	glm::vec2& shot_rel_pos = upper_barrel ? animation.upper_shot_fire_pos : animation.lower_shot_fire_pos;
	save_enemy_id = enemy.id;
	shot_rel_pos = (upper_barrel ? perp : -perp);
	
}

void TwinGun::shoot_logic(int x_id, int y_id, IEnemy& enemy) {
	if (state == State::Ready) {
		animation.start_barrel_animation(true);
		shot(x_id, y_id, enemy, true);
		cd_interleaved = 0; //уходим на cd для подготовки ко второму выстрелу.
		state = State::InterleavedCD;
	}
	else if (state == State::InterleavedReady) {
		animation.start_barrel_animation(false);
		shot(x_id, y_id, enemy, false);
		cd_shot = 0; //вся башня уходит на перезарядку.
		state = State::ShotCD;
	}
}