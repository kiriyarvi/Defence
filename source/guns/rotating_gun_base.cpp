#include "guns/rotating_gun_base.h"
#include "enemy_manager.h"
#include "covering_database.h"
#include "glm/glm.hpp"

void IRotatingGun::draw(sf::RenderWindow& window, int x_id, int y_id) {
	sf::Sprite sprite;
	sprite.setPosition(32 * x_id, 32 * y_id);
	sprite.setTexture(TextureManager::Instance().textures[base_texture]);
	window.draw(sprite);
}

void IRotatingGun::logic(double dtime_microseconds, int x_id, int y_id) {
	glm::vec2 gun_pos = glm::vec2(x_id * 32 + 16, y_id * 32 + 16);

	IEnemy* captured_enemy = nullptr;

	if (m_is_enemy_captured) {
		// если враг был ранее захвачен, то нужно проверить, жив ли он до сих пор.
		captured_enemy = EnemyManager::Instance().get_enemy_by_id(m_captured_enemy_id);
		if (!captured_enemy || !CoveringDataBase::Instance().is_available_taget(captured_enemy->id)) { // захваченный враг пропал, видимо он был удален с поля боя или же он замаскирован
			m_is_enemy_captured = false;
			captured_enemy = nullptr;
			if (m_is_gun_pointed) {
				on_gun_unpointed();
				m_is_gun_pointed = false;
			}
		}
		else {
			double dist = glm::length(captured_enemy->get_position() - gun_pos);
			if (dist > radius * 32) { // враг вне зоны досигаемости или скрыт
				m_is_enemy_captured = false;
				captured_enemy = false;
				if (m_is_gun_pointed) {
					on_gun_unpointed();
					m_is_gun_pointed = false;
				}
			}
		}
	}

	// если нет захваченного врага, найдем его.
	if (!m_is_enemy_captured) {
		// захватываем цель. Ищем ближайшего врага.
		auto& enemies = EnemyManager::Instance().m_enemies;
		if (enemies.empty()) return; // врагов нет
		
		// ищем ближайшего врага в радиусе действия (по приоритету)
		double best_priority = 0;
		for (auto& enemy : enemies) {
            if (!CoveringDataBase::Instance().is_available_taget(enemy->id))
                continue;
            auto status = get_enemy_status(*enemy.get());
            if (!status.valid)
                continue;
            double dist = glm::length(enemy->get_position() - gun_pos);
			if (dist <= radius * 32) {
                float p = status.priority;
                if (status.mult_by_distance)
                    p *= 1. - dist / (radius * 32);
				if (!captured_enemy) {
					captured_enemy = enemy.get();
                    best_priority = p;
				}
				else if (best_priority < p) {
					captured_enemy = enemy.get();
                    best_priority = p;
				}
			}
		}
		if (captured_enemy) {
			m_is_enemy_captured = true;
			m_captured_enemy_id = captured_enemy->id;
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
			if (!m_is_gun_pointed) {
				on_gun_pointed();
				m_is_gun_pointed = true;
			}
			shoot_logic(x_id, y_id, *captured_enemy);
		}
		else {
			rotation += glm::sign(angle_distance) * angle_potential;
			if (m_is_gun_pointed) {
				on_gun_unpointed();
				m_is_gun_pointed = false;
			}
		}
	}
}
