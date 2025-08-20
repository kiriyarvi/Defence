#include "enemies/solder.h"
#include "enemy_manager.h"

#include <iostream>

Solder::Solder(): m_animation(2) {
	full_health = health = 15;
	speed = 0.25;
	m_solder_sprite.setTexture(EnemyManager::Instance().enemy_textures[EnemyTexturesID::SolderWalkAnimation]);
	m_solder_sprite.setOrigin(16, 16);
	m_solder_sprite.setScale(0.25, 0.25);
	m_animation.add_framer(16, [&](int frame) {
		m_solder_sprite.setTextureRect(sf::IntRect(32 * (frame % 4), 32 * (frame / 4), 32, 32));
	});
	m_solder_ammunition.setTexture(EnemyManager::Instance().enemy_textures[EnemyTexturesID::SolderAmmunition]);
	m_solder_ammunition.setOrigin(16, 16);
	m_solder_ammunition.setScale(0.25, 0.25);
	m_animation.set_loop(true);
	m_animation.start();

	m_health_indicator.width = 8;
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
	m_health_indicator.draw(window, position.x, position.y - 8, full_health, health);
}

DestroyedEnemy Solder::get_destroyed_enemy() {
	DestroyedEnemy de;
	de.sprite = m_solder_sprite;
	return de;
}