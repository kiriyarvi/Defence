#include "enemy_manager.h"
#include "sound_manager.h"
#include "game_state.h"
#include <glm/glm.hpp>

#include "enemies/simple_enemy.h"
#include "enemies/solder.h"
#include "enemies/cruiser_I.h"

Smoke::Smoke(const glm::vec2& pos, float r, float duration):
    m_pos{ pos }, m_max_radius{ r }, m_duration{ duration }, m_animation(duration)
{
    m_circle_shape.setPointCount(20);
    m_circle_shape.setPosition(pos.x, pos.y);
}

float triangle(float x) {
    return (sin(x) + 1.) / 2.;
}

void Smoke::init_animation() {
    auto& spraying_animation = m_animation.add_subanimation(0., 0.15f * m_duration, Animation());
    auto& fade_animation = m_animation.add_subanimation(0.6f * m_duration, m_duration, Animation());

    spraying_animation.on_start = [&]() { m_enabled = true; };
    spraying_animation.on_progress = [&](double p) {
        m_current_radius = m_max_radius * p;
    };
    m_animation.on_progress = [&](double p) {
        float angle_speed = 5;
        m_rotation_1 = p * m_duration * angle_speed;
        m_rotation_2 = -p * m_duration * angle_speed;
    };
    fade_animation.on_progress = [&](double p) {
        m_fade_1 = m_fade_2 = 0.5 - 0.5 * p;
    };
    fade_animation.on_end = [&]() { m_enabled = false; };
    m_animation.start();
}

bool Smoke::logic(double dtime) {
    if (!m_animation.started())
        init_animation();
    else
        m_animation.logic(dtime);
    return m_animation.started();
}

void Smoke::draw(sf::RenderWindow& window) {
    if (!m_enabled) return;
    m_circle_shape.setRadius(m_current_radius * 32);
    m_circle_shape.setOrigin(m_current_radius * 32, m_current_radius * 32);
    m_circle_shape.setTexture(&TextureManager::Instance().textures[TextureID::Smoke1]);
    m_circle_shape.setFillColor(sf::Color(255, 255, 255, 255.f * m_fade_1));
    m_circle_shape.setRotation(m_rotation_1);
    window.draw(m_circle_shape);
    m_circle_shape.setTexture(&TextureManager::Instance().textures[TextureID::Smoke2]);
    m_circle_shape.setFillColor(sf::Color(255, 255, 255, 255.f * m_fade_2));
    m_circle_shape.setRotation(m_rotation_2);
    window.draw(m_circle_shape);
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
