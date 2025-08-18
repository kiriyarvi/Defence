#include "rotating_gun_base.h"
#include "enemy_manager.h"
#include "glm/glm.hpp"

void IRotatingGun::draw(sf::RenderWindow& window, int x_id, int y_id) {
	sf::Sprite sprite;
	sprite.setPosition(32 * x_id, 32 * y_id);
	sprite.setTexture(TileMap::Instance().textures[base_texture]);
	window.draw(sprite);
}

void IRotatingGun::logic(double dtime_microseconds, int x_id, int y_id) {
	glm::vec2 gun_pos = glm::vec2(x_id * 32 + 16, y_id * 32 + 16);

	Enemy* captured_enemy = nullptr;

	if (is_enemy_captured) {
		// если враг был ранее захвачен, то нужно проверить, жив ли он до сих пор.
		captured_enemy = EnemyManager::Instance().get_enemy_by_id(captured_enemy_id);
		if (!captured_enemy) { // захваченный враг пропал, видимо он был удален с поля боя.
			is_enemy_captured = false;
			captured_enemy = false;
		}
		else {
			double dist = glm::length(captured_enemy->get_position() - gun_pos);
			if (dist > radius * 32) { // враг вне зоны досигаемости.
				is_enemy_captured = false;
				captured_enemy = false;
			}
		}
	}

	// если нет захваченного врага, найдем его.
	if (!is_enemy_captured) {
		// захватываем цель. Ищем ближайшего врага.
		auto& enemies = EnemyManager::Instance().m_enemies;
		if (enemies.empty()) return; // врагов нет
		
		// ищем ближайшего врага в радиусе действия
		double best_dist = 0;
		for (auto& enemy : enemies) {
			double dist = glm::length(enemy.get_position() - gun_pos);
			if (dist <= radius * 32) {
				if (!captured_enemy) {
					captured_enemy = &enemy;
					best_dist = dist;
				}
				else if (best_dist > dist) {
					captured_enemy = &enemy;
					best_dist = dist;
				}
			}
		}
		if (captured_enemy) {
			is_enemy_captured = true;
			captured_enemy_id = captured_enemy->id;
		}
	}
	// если враг захвачен, нужно осуществить поворот к нему.
	if (captured_enemy) {
		glm::vec2 enemy_pos = captured_enemy->get_position();
		glm::vec2 direction = enemy_pos - gun_pos;
		double goal_angle = glm::degrees(glm::atan(direction.y, direction.x));
		double angle_potential = dtime_microseconds * rotation_speed / (1000 * 1000);
		double angle_distance = goal_angle - rotation;
		if (angle_distance >= 180)
			rotation += 360;
		if (angle_distance <= -180)
			rotation -= 360;
		angle_distance = goal_angle - rotation;

		if (angle_potential > glm::abs(angle_distance)) {
			rotation = goal_angle;
			shoot_logic(x_id, y_id, *captured_enemy);
		}
		else
			rotation += glm::sign(angle_distance) * angle_potential;
	}
}