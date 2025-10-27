#pragma once
#include "tile_map.h"
#include "SFML/Audio.hpp"
#include "enemies/IEnemy.h"
#include "wave_controller.h"

#include "utils/animation.h"

class Smoke {
public:
    Smoke(const glm::vec2& pos, float r, float duration);
    bool logic(double dtime);
    void draw(sf::RenderWindow& window);
private:
    void init_animation();
private:
    Animation m_animation;
    float m_duration;
    float m_max_radius;

    float m_current_radius = 0.0;
    float m_fade_1 = 0.5;
    float m_fade_2 = 0.5;
    float m_rotation_1 = 0;
    float m_rotation_2 = 0;
    bool m_enabled = false;

    glm::vec2 m_pos;
    sf::CircleShape m_circle_shape;
};

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
    void draw_smokes(sf::RenderWindow& window);
    void start_wave() { if (m_wave_controller) m_wave_controller->start_wave(); }
	IEnemy* get_enemy_by_id(uint32_t id);
    RoadGraph::Paths all_paths;
	std::vector<IEnemy::Ptr> m_enemies;
    void generate_waves();
    void add_smoke(Smoke&& smoke) { m_smokes.push_back(std::move(smoke)); }
private:
	EnemyManager();
    std::list<Smoke> m_smokes;
	uint32_t current_max_id = 0;
	std::list<IDestroyedEnemy::Ptr> m_destroyed_enemies;
    std::unique_ptr<WaveController> m_wave_controller;
};
