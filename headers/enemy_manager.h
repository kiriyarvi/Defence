#pragma once
#include "tile_map.h"
#include "SFML/Audio.hpp"
#include "enemies/IEnemy.h"
#include "wave_controller.h"

#include "utils/animation.h"

#include "glm/gtc/random.hpp"


class Smoke {
public:
    Smoke(const glm::vec2& pos, float r, float duration);
    bool logic(double dtime);
    void draw(sf::RenderWindow& window);
private:
    Animation m_animation;
    float m_duration;
    float m_max_radius;

    bool m_enabled = false;
    size_t m_max_particles = 70;
    glm::vec2 m_pos;

    sf::Sprite m_particle_sprite;



    float m_global_timer = 0.0;


    float m_particle_produce_duration = 0.0;
    float m_particle_flow_duration = 0.0;
    float m_particle_fade_duration = 0.0;


    struct Particle {
        Particle(const glm::vec2& vel) : m_pos{0,0} {
            m_vel = vel;
            texture_id = rand() % 16;
            m_tor_vel = glm::linearRand(-1.0, 1.0);
        }
        glm::vec2 m_pos;
        glm::vec2 m_vel; //собственная скорость частицы, суммируется с полем.
        float m_scale = 0.0;
        int texture_id = 0;
        float m_timer = 0.0;
        float m_fade = 1.0;
        float m_tor_vel = 0.0;
        float m_rot = 0.0;
    };
    std::vector<Particle> m_particles;

    std::vector<std::vector<glm::vec2>> m_curl_noise; //TODO временно.

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
