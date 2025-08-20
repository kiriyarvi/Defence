#include "enemies/IEnemy.h"
#include "game_state.h"
#include "enemy_manager.h"

void HealthIndicator::draw(sf::RenderWindow& window, float x, float y, float max_healf, float current_healf) {
	sf::RectangleShape rectangle(sf::Vector2f(width, 1));
	rectangle.setPosition(x, y);
	rectangle.setFillColor(sf::Color::Transparent);
	rectangle.setOutlineColor(sf::Color::Black);
	rectangle.setOutlineThickness(0.5f);
	rectangle.setOrigin(width / 2, 0.5);
	window.draw(rectangle);
	rectangle.setFillColor(sf::Color::Red);
	rectangle.setOutlineColor(sf::Color::Transparent);
	rectangle.setSize(sf::Vector2f(width * current_healf / max_healf, 1.f));
	rectangle.setOrigin(0, 0.5);
	rectangle.move(-width / 2, 0);
	window.draw(rectangle);
}

float DestroyedEnemy::compute_k() {
	return animation_time / (animation_duration * 1000 * 1000);
}

void DestroyedEnemy::draw(sf::RenderWindow& window) {
	float k = compute_k();
	if (k > 0.5) {
		float t = (k - 0.5) / 2;
		sprite.setColor(sf::Color(255, 255, 255, (1.f - t) * 255.f));
	}
	window.draw(sprite);
	if (k < 0.3) {
		sf::Sprite fire;
		k /= 0.3;
		int frame = k * 8;
		if (frame < 4) {
			fire.setTexture(EnemyManager::Instance().enemy_textures[EnemyTexturesID::MedBlustOfDestruction1]);
			fire.setTextureRect(sf::IntRect(16 * (frame % 2), 16 * (frame / 2), 16, 16));
		}
		else {
			frame -= 4;
			fire.setTexture(EnemyManager::Instance().enemy_textures[EnemyTexturesID::MedBlustOfDestruction2]);
			fire.setTextureRect(sf::IntRect(16 * (frame % 2), 16 * (frame / 2), 16, 16));
		}
		fire.setOrigin(8, 8);
		fire.setPosition(sprite.getPosition());
		window.draw(fire);
	}
}

void DestroyedEnemy::logic(double dtime_microseconds) {
	animation_time += dtime_microseconds;
}

bool DestroyedEnemy::is_ready() {
	return animation_time >= animation_duration * 1000 * 1000;
}

bool IEnemy::logic(double dtime) {
	sf::Vector2f current_pos(position.x, position.y);
	auto sf_dir = goal - current_pos;
	glm::vec2 dir = { sf_dir.x, sf_dir.y };
	float dist = glm::length(dir);
	dir = dir / dist;
	float potential = dtime * speed * 32;
	if (dist * 1000 * 1000 > potential) {
		float angle = glm::degrees(glm::atan(dir.y, dir.x));
		rotation = angle;
		dir = dir * potential / (1000 * 1000.f);
		position += glm::vec2(dir.x, dir.y);
	}
	else { // при старте goal нулевой и current_pos нулевой, поэтому попадаем сюда и выставляем начальную точку маршрута. 
		auto& enemy_manager = EnemyManager::Instance();
		auto& path = enemy_manager.all_paths[path_id];
		if (goal_path_node + 1 == path.size()) {
			GameState::Instance().player_health_add(-1); //TODO отнимать cost
			path_is_completed = true;
			return true;
		}
		position = glm::vec2(path[goal_path_node]->x * 32 + 16, path[goal_path_node]->y * 32 + 16);
		++goal_path_node;
		goal = sf::Vector2f(path[goal_path_node]->x * 32 + 16, path[goal_path_node]->y * 32 + 16);
	}
	return false;
}