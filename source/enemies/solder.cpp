#include "enemies/solder.h"
#include "enemy_manager.h"

#include <iostream>

Solder::Solder(): m_animation(2), IEnemy(ParamsManager::Instance().params.enemies.solder, EnemyType::Solder,Collision(glm::vec2(-3.25,-2.75), glm::vec2(6, 2))) {
	m_solder_sprite.setTexture(TextureManager::Instance().textures[TextureID::SolderWalkAnimation]);
	m_solder_sprite.setOrigin(16, 16);
	m_solder_sprite.setScale(0.25, 0.25);
	m_animation.add_framer(16, [&](int frame) {
		m_solder_sprite.setTextureRect(sf::IntRect(32 * (frame % 4), 32 * (frame / 4), 32, 32));
	});
	m_solder_ammunition.setTexture(TextureManager::Instance().textures[TextureID::SolderAmmunition]);
	m_solder_ammunition.setOrigin(16, 16);
	m_solder_ammunition.setScale(0.25, 0.25);
	m_animation.set_loop(true);
	m_animation.start();

	m_health_indicator.width = 8;
    infantry = true;
    wheels = Wheels::None;
}

bool Solder::logic(double dtime) {
	m_animation.logic(dtime);
	return IEnemy::logic(dtime);
}

void Solder::draw(sf::RenderWindow& window) {
	m_solder_sprite.setPosition(position.x, position.y);
	m_solder_sprite.setRotation(rotation);
	m_solder_ammunition.setPosition(position.x, position.y);
	m_solder_ammunition.setRotation(rotation);
	window.draw(m_solder_sprite);
	window.draw(m_solder_ammunition);
	m_health_indicator.draw(window, position.x, position.y - 8, params.health, health);
}

void Solder::draw_effects(sf::RenderWindow& window) {
    IEnemy::draw_effects(window);
    m_health_indicator.draw(window, position.x, position.y - 8, params.health, health);
}

class DestroyedSolder: public IDestroyedEnemy {
public:
	DestroyedSolder() {
		m_animation.set_duration(2.5);
		auto [_, fade] = m_animation.split(0.3);
		fade->on_progress = [&](double progress) {
			sprite.setColor(sf::Color(255, 255, 255, (1. - progress) * 255.f));
		};
		m_animation.start();
	}
	void draw(sf::RenderWindow& window) override { window.draw(sprite); }
	void logic(double dtime_microseconds) override { m_animation.logic(dtime_microseconds); }
	bool is_ready() override  { return !m_animation.started(); }
	sf::Sprite sprite;
private:
	Animation m_animation;
};

IDestroyedEnemy::Ptr Solder::get_destroyed_enemy() {
	auto de = std::make_unique<DestroyedSolder>();
	de->sprite = m_solder_sprite;
	de->sprite.setTexture(TextureManager::Instance().textures[TextureID::DeadSolder]);
	de->sprite.setOrigin(32, 32);
	de->sprite.setTextureRect(sf::IntRect(0, 0, 64, 64));
	return de;
}
