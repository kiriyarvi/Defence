#include "enemies/simple_enemy.h"

SimpleEnemy::SimpleEnemy(EnemyTexturesID enemy_texture, EnemyTexturesID destroyed_enemy_texture):
	m_destroyed_enemy_texture(destroyed_enemy_texture){
	m_enemy_sprite.setTexture(EnemyManager::Instance().enemy_textures[enemy_texture]);
	m_enemy_sprite.setOrigin(16, 16);
}

void SimpleEnemy::draw(sf::RenderWindow& window) {
	m_enemy_sprite.setPosition(position.x, position.y);
	m_enemy_sprite.setRotation(rotation);
	window.draw(m_enemy_sprite);
	m_indicator.draw(window, position.x, position.y - 8, full_health, health);
}

DestroyedEnemy SimpleEnemy::get_destroyed_enemy() {
	DestroyedEnemy de;
	de.sprite = m_enemy_sprite;
	de.sprite.setTexture(EnemyManager::Instance().enemy_textures[m_destroyed_enemy_texture]);
	return de;
}