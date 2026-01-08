#include "enemy_manager.h"
#include "sound_manager.h"
#include "resource_manager.h"
#include "covering_database.h"
#include "game_state.h"
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include "enemies/simple_enemy.h"
#include "enemies/solder.h"
#include "enemies/cruiser_I.h"



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
        break;
    case EnemyType::SmokeTruck:
        m_enemies.push_back(std::make_unique<SmokeTruck>());
        break;
    }
    if (boss)
        m_enemies.back()->make_boss();
    m_enemies.back()->path_id = path_id;
	m_enemies.back()->id = ++current_max_id;
    auto& path = all_paths[path_id.start_node][path_id.path];
    m_enemies.back()->position = glm::vec2(path[0]->x * 32 + 16, path[0]->y * 32 + 16);
    m_enemies.back()->goal_path_node = 1;
    m_enemies.back()->goal = sf::Vector2f(path[1]->x * 32 + 16, path[1]->y * 32 + 16);
	m_enemies.back()->logic(0.0); // чтобы установить статус маскировки и goal.
	if (current_max_id > 32768)
		current_max_id = 0;
}

void EnemyManager::logic(double dtime) {
	std::vector<IEnemy::Ptr> new_enemies;
	for (auto& enemy : m_enemies) { /// проходим по врагам и удаляем уничтоженных.
		if (enemy->health <= 0) { 
            GameState::Instance().enemy_defeated(enemy->type); /// для статистики.
			m_destroyed_enemies.push_back(enemy->get_destroyed_enemy());
			GameState::Instance().player_coins_add(enemy->params.reward); /// награда.
		}
		else {
			new_enemies.push_back(std::move(enemy));
		}
	}
	m_enemies = std::move(new_enemies); /// обновление врагов.
	for (auto& enemy : m_enemies)
		if (enemy->logic(dtime)); /// логика врагов, передвижение по маршруту.
	m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
		[](const IEnemy::Ptr& enemy) {return enemy->path_is_completed; }),
		m_enemies.end()
	); /// удаляем тех врагов, которые достигли конца маршрута

    /// У уничтоженных врагов есть анимации уничтожения, логику все равно нужно вызывать.
	for (auto& destroyed_enemy : m_destroyed_enemies)
		destroyed_enemy->logic(dtime);
	m_destroyed_enemies.remove_if([](IDestroyedEnemy::Ptr& enemy) { return enemy->is_ready(); });
    /// Логика волн
    if (m_wave_controller) {
        m_wave_controller->logic(dtime);
        if (m_enemies.empty()) {
            if (!m_wave_controller->next_wave()) {
                GameState::Instance().win();
            }
        }
    }
    /// Анимация дымовых завес.
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

void EnemyManager::draw_effects(sf::RenderWindow& window) {
    for (auto& e : m_enemies)
        e->draw_effects(window);
    for (auto& smoke : m_smokes)
        smoke.draw(window);
    for (auto& e : m_enemies)
        e->post_smoke_effects(window);
}

IEnemy* EnemyManager::get_enemy_by_id(uint32_t id) {
	auto it = std::find_if(m_enemies.begin(), m_enemies.end(), [id](const IEnemy::Ptr& enemy) {return enemy->id == id; });
	return it == m_enemies.end() ? nullptr : it->get();
}
