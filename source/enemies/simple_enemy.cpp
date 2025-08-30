#include "enemies/simple_enemy.h"
#include "glm/gtc/random.hpp"
#include "shader_manager.h"
#include "game_state.h"

SimpleEnemy::SimpleEnemy(TextureID enemy_texture, TextureID destroyed_enemy_texture, Sounds destruction_sound, const ParamsManager::Params::Enemies::Enemy& params, EnemyType t):
	m_destroyed_enemy_texture(destroyed_enemy_texture),
	m_destruction_sound(destruction_sound),
	IEnemy(params, t)
{
	m_enemy_sprite.setTexture(TextureManager::Instance().textures[enemy_texture]);
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
	de->destroyed_enemy_sprite.setTexture(TextureManager::Instance().textures[m_destroyed_enemy_texture]);
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
	de->destroyed_enemy_sprite.setTexture(TextureManager::Instance().textures[m_destroyed_enemy_texture]);
	de->destroyed_enemy_sprite.setScale(0.3, 0.3);
	return de;
}


BTR::BTR(): IEnemy(ParamsManager::Instance().params.enemies.BTR, EnemyType::BTR) {
    wheels = Wheels::Tracks;
    infantry = false;
    m_btr.sprite.setTexture(TextureManager::Instance().textures[TextureID::BTR]);
    m_btr.set_position_origin(16, 16);
    m_btr.set_rotation_origin(16, 16);
    m_upper_truck = &m_btr.childs.emplace_back();
    m_lower_truck = &m_btr.childs.emplace_back();

    auto& shader = ShaderManager::Instance().shaders[Shader::Scroll];

    m_upper_truck->sprite.setTexture(TextureManager::Instance().textures[TextureID::Trucks]);
    m_upper_truck->sprite.setTextureRect(sf::IntRect(0, 0, 60, 7));
    m_upper_truck->sprite.setScale(2 / 7.f, 2 / 7.f);
    m_upper_truck->set_position(9, 10);
    m_upper_truck->shader = &shader;

    m_lower_truck->sprite.setTexture(TextureManager::Instance().textures[TextureID::Trucks]);
    m_lower_truck->sprite.setTextureRect(sf::IntRect(0, 0, 60, 7));
    m_lower_truck->sprite.setScale(2 / 7.f, 2 / 7.f);
    m_lower_truck->set_position(9, 20);
    m_lower_truck->shader = &shader;

}

void BTR::draw(sf::RenderWindow& window) {
    auto& shader = ShaderManager::Instance().shaders[Shader::Scroll];
    shader.setUniform("texture", sf::Shader::CurrentTexture);
    shader.setUniform("offset", -(float)m_trucks_offset / (1000 * 1000));
    m_btr.set_rotation(rotation);
    m_btr.set_position(position.x, position.y);
    m_btr.draw(window);
    m_indicator.draw(window, position.x, position.y - 8, params.health, health);
}

bool BTR::logic(double dtime_microseconds) {
    m_trucks_offset += params.speed * dtime_microseconds;
    if (m_trucks_offset >= 1000 * 1000)
        m_trucks_offset = 0;
    return IEnemy::logic(dtime_microseconds);
}

IDestroyedEnemy::Ptr BTR::get_destroyed_enemy() {
    auto de = std::make_unique<SimpleEnemyDestroyed>(std::make_unique<DenceBlustFramer>(), 1.4, 2.0, Sounds::DenceBlust);
    de->destroyed_enemy_sprite;
    de->destroyed_enemy_sprite.setOrigin(16, 16);
    de->destroyed_enemy_sprite.setPosition(position.x, position.y);
    de->destroyed_enemy_sprite.setRotation(rotation);
    de->destroyed_enemy_sprite.setTexture(TextureManager::Instance().textures[TextureID::BTRDestroyed]);
    return de;
}
