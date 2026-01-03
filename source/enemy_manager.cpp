#include "enemy_manager.h"
#include "sound_manager.h"
#include "game_state.h"
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include "enemies/simple_enemy.h"
#include "enemies/solder.h"
#include "enemies/cruiser_I.h"

#include <fstream> //TODO временно.

Smoke::Smoke(const glm::vec2& pos, float r, float duration):
    m_pos{ pos }, m_max_radius{ r * 32 }, m_duration{ duration }, m_animation(duration)
{
    m_particle_sprite.setTexture(TextureManager::Instance().textures[TextureID::SmokeParticles]);
    m_particle_produce_duration = 0.2f * m_duration * 1000 * 1000;
    m_particle_flow_duration = 0.5f * m_duration * 1000 * 1000;
    m_particle_fade_duration = 0.3f * m_duration * 1000 * 1000;
    m_enabled = true;


    std::ifstream file("noises/curl_noise.bin", std::ios::binary);
    uint32_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));

    m_curl_noise.resize(size, std::vector<glm::vec2>(size));
    for (auto& row : m_curl_noise) {
        file.read(reinterpret_cast<char*>(row.data()),
            row.size() * sizeof(glm::vec2));
    }

}

float smoothstep(float x) {
    return x * x * (3 - 2 * x);
}

bool Smoke::logic(double dtime) {
    m_global_timer += dtime;
    // +m_particle_produce_duration чтобы последняя частица успела испариться.
    if (m_global_timer >= m_duration * 1000.f * 1000.f + m_particle_produce_duration)
        return false;
    float dp = dtime / m_particle_produce_duration;
    for (auto& particle : m_particles) {
        particle.m_timer += dtime;
        float curl_fade = 1.;
        if (particle.m_timer <= m_particle_produce_duration) {
            // учет импульса, данного при рождении частицы.
            float p = particle.m_timer / m_particle_produce_duration;
            particle.m_pos += particle.m_vel * dp * (1 - p) * (m_max_radius * 2);
            particle.m_scale = glm::lerp(0.25f, 1.f, p);
            curl_fade = p;
        }
        // curl-noise
        float lx = (particle.m_pos.x / m_max_radius + 1) / 2.f;
        float ly = (particle.m_pos.y / m_max_radius + 1) / 2.f;
        lx = glm::clamp(lx, 0.f, 1.f);
        ly = glm::clamp(ly, 0.f, 1.f);
        size_t nx = lx * (m_curl_noise.size() - 1) + 0.5;
        size_t ny = ly * (m_curl_noise.size() - 1) + 0.5;
        particle.m_pos += curl_fade * m_curl_noise[nx][ny] * 20.f * (float)(dtime / (1000.f * 1000.f));
        if (particle.m_timer >= m_particle_produce_duration + m_particle_flow_duration) {
            float p = glm::clamp((particle.m_timer - m_particle_produce_duration - m_particle_flow_duration) / m_particle_fade_duration, 0.f,1.f);
            particle.m_fade = smoothstep(1 - p);
        }
        particle.m_rot += 20.f * particle.m_tor_vel * (dtime / (1000.f * 1000.f));
    }

    if (m_global_timer <= m_particle_produce_duration) {
        int N = (m_global_timer / m_particle_produce_duration) * m_max_particles;
        for (size_t i = m_particles.size(); i < N; ++i)
            m_particles.push_back(glm::diskRand(1.f));
    }
    return true;
}

void Smoke::draw(sf::RenderWindow& window) {
    if (!m_enabled) return;
    for (auto it = m_particles.rbegin(); it != m_particles.rend(); ++it) {
        auto& particle = *it;
        m_particle_sprite.setPosition(m_pos.x + particle.m_pos.x, m_pos.y + particle.m_pos.y);
        m_particle_sprite.setScale(particle.m_scale, particle.m_scale);
        m_particle_sprite.setTextureRect(sf::IntRect((particle.texture_id % 4) * 128, (particle.texture_id / 4) * 128, 128, 128));
        m_particle_sprite.setOrigin(64, 64);
        m_particle_sprite.setColor(sf::Color(255, 255, 255, particle.m_fade * 255.f));
        m_particle_sprite.setRotation(particle.m_rot);
        window.draw(m_particle_sprite);
    }
    sf::CircleShape circ(m_max_radius, 40);
    circ.setOrigin(m_max_radius, m_max_radius);
    circ.setPosition(m_pos.x, m_pos.y);
    circ.setOutlineThickness(1);
    circ.setOutlineColor(sf::Color::Red);
    circ.setFillColor(sf::Color::Transparent);
    window.draw(circ);

}




EnemyManager::EnemyManager() {}

void EnemyManager::generate_waves() {
    all_paths = TileMap::Instance().get_road_graph().find_all_paths();
    m_wave_controller = std::make_unique<WaveController>();
}

void EnemyManager::spawn(EnemyType type, RoadGraph::PathID path_id, bool boss) {
    switch (type) {
    case EnemyType::Solder:
        m_enemies.push_back(std::make_unique<Solder>());
        break;
    case EnemyType::Bike:
        m_enemies.push_back(std::make_unique<Bike>());
        break;
    case EnemyType::Pickup:
        m_enemies.push_back(std::make_unique<Pickup>());
        break;
    case EnemyType::Truck:
        m_enemies.push_back(std::make_unique<Truck>());
        break;
    case EnemyType::BTR:
        m_enemies.push_back(std::make_unique<BTR>());
        break;
    case EnemyType::Tank:
        m_enemies.push_back(std::make_unique<Tank>());
        break;
    case EnemyType::CruiserI:
        m_enemies.push_back(std::make_unique<CruiserI>());
    case EnemyType::SmokeTruck:
        m_enemies.push_back(std::make_unique<SmokeTruck>());
        break;
    }
    if (boss)
        m_enemies.back()->make_boss();
    m_enemies.back()->path_id = path_id;
	m_enemies.back()->id = ++current_max_id;
	m_enemies.back()->logic(0.0); // чтобы установить верную позицию.
	if (current_max_id > 32768)
		current_max_id = 0;
}

void EnemyManager::logic(double dtime) {
	std::vector<IEnemy::Ptr> new_enemies;
	for (auto& enemy : m_enemies) {
		if (enemy->health <= 0) {
            GameState::Instance().enemy_defeated(enemy->type);
			m_destroyed_enemies.push_back(enemy->get_destroyed_enemy());
			GameState::Instance().player_coins_add(enemy->params.reward);
		}
		else {
			new_enemies.push_back(std::move(enemy));
		}
	}
	m_enemies = std::move(new_enemies);
	for (auto& enemy : m_enemies)
		if (enemy->logic(dtime));
	m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
		[](const IEnemy::Ptr& enemy) {return enemy->path_is_completed; }),
		m_enemies.end()
	);

	for (auto& destroyed_enemy : m_destroyed_enemies)
		destroyed_enemy->logic(dtime);
	m_destroyed_enemies.remove_if([](IDestroyedEnemy::Ptr& enemy) { return enemy->is_ready(); });
    if (m_wave_controller) {
        m_wave_controller->logic(dtime);
        if (m_enemies.empty()) {
            if (!m_wave_controller->next_wave()) {
                GameState::Instance().win();
            }
        }
    }
    auto it = m_smokes.begin();
    while (it != m_smokes.end()) {
        if (!it->logic(dtime))
            it = m_smokes.erase(it);
        else
            ++it;
    }
}


void EnemyManager::draw(sf::RenderWindow& window) {
	for (auto& destroyed_enemy : m_destroyed_enemies) {
		destroyed_enemy->draw(window);
	}
	
	for (auto& enemy : m_enemies)
		enemy->draw(window);

}

void EnemyManager::draw_smokes(sf::RenderWindow& window) {
    for (auto& smoke : m_smokes)
        smoke.draw(window);
}

IEnemy* EnemyManager::get_enemy_by_id(uint32_t id) {
	auto it = std::find_if(m_enemies.begin(), m_enemies.end(), [id](const IEnemy::Ptr& enemy) {return enemy->id == id; });
	return it == m_enemies.end() ? nullptr : it->get();
}
