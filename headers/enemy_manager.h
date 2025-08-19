#pragma once
#include "tile_map.h"
#include "SFML/Audio.hpp"


enum class EnemyTexturesID {
	Tank,
	TankDestroyed,
	Truck,
	TruckDestroyed,
	MedBlustOfDestruction1,
	MedBlustOfDestruction2,
	Bike
};

class HealthIndicator {
public:
	void draw(sf::RenderWindow& window, float x, float y, float max_healf, float current_healf);
	float width = 16;
};

class Enemy {
public:
	Enemy() = default;
	Enemy(const Enemy&) = delete;
	Enemy& operator=(const Enemy&) = delete;
	Enemy(Enemy&&) = default;
	Enemy& operator=(Enemy&&) = default;
	virtual void draw(sf::RenderWindow& window);
	virtual bool logic(double dtime); // true --- достиг конца пути
	virtual ~Enemy() = default;
	glm::vec2 get_position();
	float speed;
	int health;
	int full_health;
	sf::Sprite sprite;
	EnemyTexturesID destroyed_texture;
	sf::Vector2f goal; // текущая целевая точка
	int path_id = 0; // путь по которому движется враг.
	int goal_path_node = 0; // номер узла в пути, к которому враг стремиться на данный момент
	std::unique_ptr<sf::Sound> engine_sound;
	uint32_t id;
	bool path_is_completed = false;
};

class DestroyedEnemy {
public:
	sf::Sprite sprite;
	void draw(sf::RenderWindow& window);
	void logic(double dtime_microseconds);
	bool is_ready();
private:
	float compute_k();
	double animation_time = 0;
	double animation_duration = 2;
};

Enemy create_tank();
Enemy create_truck();
Enemy create_bike();

class EnemyManager {
public:
	static EnemyManager& Instance() {
		static EnemyManager instance; // Создаётся при первом вызове, потокобезопасно в C++11+
		return instance;
	}

	void spawn();

	// Удаляем копирование и перемещение
	EnemyManager(const EnemyManager&) = delete;
	EnemyManager& operator=(const EnemyManager&) = delete;
	EnemyManager(EnemyManager&&) = delete;
	EnemyManager& operator=(EnemyManager&&) = delete;
	void logic(double dtime);
	void draw(sf::RenderWindow& window);
	Enemy* get_enemy_by_id(uint32_t id);
	std::vector<std::vector<RoadGraph::Node*>> all_paths;
	std::unordered_map<EnemyTexturesID, sf::Texture> enemy_textures;
	std::vector<Enemy> m_enemies;
private:
	EnemyManager();
	uint32_t current_max_id = 0;
	std::list<DestroyedEnemy> m_destroyed_enemies;
};