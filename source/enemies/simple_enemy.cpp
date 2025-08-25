#include "enemies/simple_enemy.h"
#include "glm/gtc/random.hpp"

SimpleEnemy::SimpleEnemy(EnemyTexturesID enemy_texture, EnemyTexturesID destroyed_enemy_texture, Sounds destruction_sound, const ParamsManager::Params::Enemies::Enemy& params):
	m_destroyed_enemy_texture(destroyed_enemy_texture),
	m_destruction_sound(destruction_sound),
	IEnemy(params)
{
	m_enemy_sprite.setTexture(EnemyManager::Instance().enemy_textures[enemy_texture]);
	m_enemy_sprite.setOrigin(16, 16);
}

void SimpleEnemy::draw(sf::RenderWindow& window) {
	m_enemy_sprite.setPosition(position.x, position.y);
	m_enemy_sprite.setRotation(rotation);
	window.draw(m_enemy_sprite);
	draw_effects(window);
	m_indicator.draw(window, position.x, position.y - 8, params.health, health);
}

IDestroyedEnemy::Ptr SimpleEnemy::get_destroyed_enemy() {
	auto de = std::make_unique<SimpleEnemyDestroyed>(std::make_unique<MedBlustFramer>(), 1, 2.0, m_destruction_sound);
	de->destroyed_enemy_sprite = m_enemy_sprite;
	de->destroyed_enemy_sprite.setTexture(EnemyManager::Instance().enemy_textures[m_destroyed_enemy_texture]);
	return de;
}

SimpleEnemyDestroyed::SimpleEnemyDestroyed(ISpriteFramer::Ptr&& framer, double blust_time, double fade_time, Sounds destruction_sound): 
	m_framer(std::move(framer)) {
	m_animation.set_duration(fade_time + blust_time);
	auto [fire_animation, fade_animation] = m_animation.split(blust_time / (fade_time + blust_time));
	fire_animation->add_framer(m_framer);
	fire_animation->on_end = [&]() { m_draw_blust = false; };
	fade_animation->on_progress = [&](double progress) {
		destroyed_enemy_sprite.setColor(sf::Color(255, 255, 255, (1. - progress) * 255.f));
	};
	m_animation.start();
	SoundManager::Instance().play(destruction_sound);
}

void SimpleEnemyDestroyed::draw(sf::RenderWindow& window) {
	window.draw(destroyed_enemy_sprite);
	if (m_draw_blust) {
		m_framer->sprite.setPosition(destroyed_enemy_sprite.getPosition());
		m_framer->sprite.setRotation(destroyed_enemy_sprite.getRotation());
		window.draw(m_framer->sprite);
	}

}

void SimpleEnemyDestroyed::logic(double dtime_microseconds) {
	m_animation.logic(dtime_microseconds);
}

bool SimpleEnemyDestroyed::is_ready() {
	return !m_animation.started();
}

BikeDestroyed::BikeDestroyed() {
	std::vector<Animation*> a;
	double blust_duration = 1.;
	double blusts_interleaving = 0.2;
	double fade_duration = 2;
	m_animation.set_duration(blust_duration + 2 * blusts_interleaving + fade_duration);
	a.push_back(&m_animation.add_subanimation(0., blust_duration, Animation()));
	a.push_back(&m_animation.add_subanimation(blusts_interleaving, blust_duration + blusts_interleaving, Animation()));
	a.push_back(&m_animation.add_subanimation(2 * blusts_interleaving + 0.2, blust_duration + 2 * blusts_interleaving + 0.2, Animation()));
	a.push_back(&m_animation.add_subanimation(blust_duration + 2 * blusts_interleaving, blust_duration + 2 * blusts_interleaving + fade_duration, Animation()));
	glm::vec2 offset(0,0);
	for (int i = 0; i < 3; ++i) {
		m_blusts_info[i].framer = std::make_unique<DenceBlustFramer>();
		a[i]->add_framer(m_blusts_info[i].framer);
		a[i]->on_start = [&, i]() { 
			m_blusts_info[i].active = true;
			SoundManager::Instance().play(Sounds::DenceBlust);
		};
		a[i]->on_end = [&, i]() { m_blusts_info[i].active = false; };
		m_blusts_info[i].offset.x = offset.x;
		m_blusts_info[i].offset.y = offset.y;
		offset += glm::circularRand(8.);
	}
	auto fade = a[3];
	fade->on_progress = [&](double progress) {
		destroyed_enemy_sprite.setColor(sf::Color(255, 255, 255, (1. - progress) * 255.f));
	};

	m_animation.start();
}

void BikeDestroyed::draw(sf::RenderWindow& window) {
	window.draw(destroyed_enemy_sprite);
	for (int i = 0; i < 3; ++i) {
		auto& blust = m_blusts_info[i];
		if (blust.active) {
			blust.framer->sprite.setPosition(destroyed_enemy_sprite.getPosition() + blust.offset);
			blust.framer->sprite.setRotation(destroyed_enemy_sprite.getRotation());
			window.draw(blust.framer->sprite);
		}
	}
}

void BikeDestroyed::logic(double dtime_microseconds) {
	m_animation.logic(dtime_microseconds);
}
bool BikeDestroyed::is_ready() {
	return !m_animation.started();
}

IDestroyedEnemy::Ptr Bike::get_destroyed_enemy() {
	auto de = std::make_unique<BikeDestroyed>();
	de->destroyed_enemy_sprite = m_enemy_sprite;
	de->destroyed_enemy_sprite.setTexture(EnemyManager::Instance().enemy_textures[m_destroyed_enemy_texture]);
	de->destroyed_enemy_sprite.setScale(0.3, 0.3);
	return de;
}
