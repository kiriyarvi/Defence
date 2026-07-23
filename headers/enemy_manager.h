#pragma once
#include "tile_map.h"
#include "SFML/Audio.hpp"
#include "enemies/enemy_container.h"
#include "wave_controller.h"
#include <enemies/smoke.h>

#include "utils/animation.h"
#include <list>

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
	void logic(double dtime);
	void draw(sf::RenderWindow& window);
    void draw_effects(sf::RenderWindow& window);
    void start_wave() { if (m_wave_controller) m_wave_controller->start_wave(); }
	IEnemy* get_enemy_by_id(EnemyContainer::EnemyID id);
    RoadGraph::Paths all_paths;
    std::list<IEnemy*> MREW_enemies; //< отдельный массив для врагов с функцией радиоподавления. Гарантируется, что в этом массиве расположены не уничтоженные враги.
    void generate_waves();
    void add_smoke(Smoke&& smoke) { m_smokes.push_back(std::move(smoke)); }
    const std::list<Smoke>& get_smokes() { return m_smokes; }
    EnemyContainer& get_enemy_container() { return m_enemies; }
private:
	EnemyManager();
private:
    EnemyContainer m_enemies;
    std::list<Smoke> m_smokes;
	std::list<IDestroyedEnemy::Ptr> m_destroyed_enemies;
    std::unique_ptr<WaveController> m_wave_controller;
};
