#pragma once
#include "tile_map.h"
#include "SFML/Audio.hpp"
#include "enemies/IEnemy.h"
#include "wave_controller.h"


class EnemyManager {
public:
	static EnemyManager& Instance() {
		static EnemyManager instance; // Создаётся при первом вызове, потокобезопасно в C++11+
		return instance;
	}

	void spawn(EnemyType type, RoadGraph::PathID id, bool boss = false);

	// Удаляем копирование и перемещение
	EnemyManager(const EnemyManager&) = delete;
	EnemyManager& operator=(const EnemyManager&) = delete;
	EnemyManager(EnemyManager&&) = delete;
	EnemyManager& operator=(EnemyManager&&) = delete;
	void logic(double dtime); // если возвращает true --- спавнеры кончились.
	void draw(sf::RenderWindow& window);
    void start_wave() { if (m_wave_controller) m_wave_controller->start_wave(); }
	IEnemy* get_enemy_by_id(uint32_t id);
    RoadGraph::Paths all_paths;
	std::vector<IEnemy::Ptr> m_enemies;
    void generate_waves();
private:
	EnemyManager();
	uint32_t current_max_id = 0;
	std::list<IDestroyedEnemy::Ptr> m_destroyed_enemies;
    std::unique_ptr<WaveController> m_wave_controller;
};
