#include "enemy_manager.h"
#include "sound_manager.h"
#include <glm/glm.hpp>


EnemyManager::EnemyManager() {
	all_paths = TileMap::Instance().get_road_graph().find_all_paths();
	enemy_textures[EnemyTexturesID::Tank].loadFromFile("sprites/tank.png");
	enemy_textures[EnemyTexturesID::TankDestroyed].loadFromFile("sprites/tank_destroyed.png");
	enemy_textures[EnemyTexturesID::Truck].loadFromFile("sprites/truck.png");
	enemy_textures[EnemyTexturesID::TruckDestroyed].loadFromFile("sprites/truck_destroyed.png");
	enemy_textures[EnemyTexturesID::MedBlustOfDestruction1].loadFromFile("sprites/med_blust_of_destruction1.png");
	enemy_textures[EnemyTexturesID::MedBlustOfDestruction2].loadFromFile("sprites/med_blust_of_destruction2.png");
}

void EnemyManager::spawn() {
	m_enemies.push_back((rand() % 2 ? create_tank() : create_truck()));
	m_enemies.back().path_id = rand() % all_paths.size();
	m_enemies.back().id = ++current_max_id;
	if (current_max_id > 32768)
		current_max_id = 0;
}

void EnemyManager::logic(double dtime) {
	std::vector<Enemy> new_enemies;
	for (auto& enemy : m_enemies) {
		if (enemy.health <= 0) {
			DestroyedEnemy de;
			de.sprite = enemy.sprite;
			de.sprite.setTexture(enemy_textures[enemy.destroyed_texture]);
			m_destroyed_enemies.push_back(de);
			SoundManager::Instance().play(Sounds::MedBlustOfDestruction);
		}
		else {
			new_enemies.push_back(std::move(enemy));
		}
	}
	m_enemies = std::move(new_enemies);
	for (auto& enemy : m_enemies) {
		if (enemy.logic(dtime));
			//TODO
	}
	for (auto& destroyed_enemy : m_destroyed_enemies)
		destroyed_enemy.logic(dtime);
	m_destroyed_enemies.remove_if([](DestroyedEnemy& enemy) { return enemy.is_ready(); });
}


void EnemyManager::draw(sf::RenderWindow& window) {
	for (auto& destroyed_enemy : m_destroyed_enemies) {
		destroyed_enemy.draw(window);
	}
	
	for (auto& enemy : m_enemies) {
		enemy.draw(window);
	}

}

Enemy* EnemyManager::get_enemy_by_id(uint32_t id) {
	auto it = std::find_if(m_enemies.begin(), m_enemies.end(), [id](Enemy& enemy) {return enemy.id == id; });
	return it == m_enemies.end() ? nullptr : &(*it);
}

Enemy create_tank() {
	Enemy enemy;
	enemy.speed = 0.4;
	enemy.full_health = enemy.health = 100;
	auto& enemy_manager = EnemyManager::Instance();
	enemy.sprite = sf::Sprite(enemy_manager.enemy_textures[EnemyTexturesID::Tank]);
	enemy.sprite.setOrigin(16, 16);

	enemy.engine_sound = std::make_unique<sf::Sound>();
	enemy.engine_sound->setLoop(true);
	enemy.engine_sound->setBuffer(SoundManager::Instance().sounds[Sounds::TankEngine]);
	enemy.engine_sound->play();
	enemy.engine_sound->setVolume(10.f);
	enemy.destroyed_texture = EnemyTexturesID::TankDestroyed;
	return enemy;
}

Enemy create_truck() {
	Enemy enemy;
	enemy.speed = 0.8;
	enemy.full_health = enemy.health = 50;
	auto& enemy_manager = EnemyManager::Instance();
	enemy.sprite = sf::Sprite(enemy_manager.enemy_textures[EnemyTexturesID::Truck]);
	enemy.sprite.setOrigin(16, 16);

	enemy.engine_sound = std::make_unique<sf::Sound>();
	enemy.engine_sound->setLoop(true);
	enemy.engine_sound->setBuffer(SoundManager::Instance().sounds[Sounds::TankEngine]);
	enemy.engine_sound->play();
	enemy.engine_sound->setVolume(10.f);
	enemy.destroyed_texture = EnemyTexturesID::TruckDestroyed;
	return enemy;
}


void Enemy::draw(sf::RenderWindow& window) {
	window.draw(sprite);
	HealthIndicator ind;
	ind.width = 16;
	auto offset = sprite.getPosition() + sf::Vector2f{ 0, -8 };
	ind.draw(window, offset.x, offset.y, full_health, health);
}

bool Enemy::logic(double dtime) {
	sf::Vector2f current_pos = sprite.getPosition();
	auto sf_dir = goal - current_pos;
	glm::vec2 dir = { sf_dir.x, sf_dir.y };
	float dist = glm::length(dir);
	dir = dir / dist;
	float potential = dtime * speed * 32;
	if (dist * 1000 * 1000 > potential) {
		float angle = glm::degrees(glm::atan(dir.y, dir.x));
		sprite.setRotation(angle);
		dir = dir * potential / (1000 * 1000.f);
		sprite.move(dir.x, dir.y);
	}
	else { // при старте goal нулевой и current_pos нулевой, поэтому попадаем сюда и выставляем начальную точку маршрута. 
		auto& enemy_manager = EnemyManager::Instance();
		auto& path = enemy_manager.all_paths[path_id];
		if (goal_path_node + 1 == path.size())
			return true;
		sprite.setPosition(path[goal_path_node]->x * 32 + 16, path[goal_path_node]->y * 32 + 16);
		++goal_path_node;
		goal = sf::Vector2f(path[goal_path_node]->x * 32 + 16, path[goal_path_node]->y * 32 + 16);
	}
	return false;
}

glm::vec2 Enemy::get_position() {
	auto pos = sprite.getPosition();
	return glm::vec2(pos.x, pos.y);
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

