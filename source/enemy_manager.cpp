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
    all_paths = GameState::Instance().get_map().get_road_graph().find_all_paths();
    m_wave_controller = std::make_unique<WaveController>();
    m_enemies.create(200); //TODO
}

void EnemyManager::spawn(EnemyType type, RoadGraph::PathID path_id, bool boss) {
    IEnemy* enemy = nullptr;
    switch (type) {
    case EnemyType::Solder:
        enemy = m_enemies.add_enemy(std::make_unique<Solder>());
        break;
    case EnemyType::Bike:
        enemy = m_enemies.add_enemy(std::make_unique<Bike>());
        break;
    case EnemyType::Pickup:
        enemy = m_enemies.add_enemy(std::make_unique<Pickup>());
        break;
    case EnemyType::Truck:
        enemy = m_enemies.add_enemy(std::make_unique<Truck>());
        break;
    case EnemyType::BTR:
        enemy = m_enemies.add_enemy(std::make_unique<BTR>());
        break;
    case EnemyType::Tank:
        enemy = m_enemies.add_enemy(std::make_unique<Tank>());
        break;
    case EnemyType::CruiserI:
        enemy = m_enemies.add_enemy(std::make_unique<CruiserI>());
        break;
    case EnemyType::SmokeTruck:
        enemy = m_enemies.add_enemy(std::make_unique<SmokeTruck>());
        break;
    case EnemyType::MREW:
        enemy = m_enemies.add_enemy(std::make_unique<MREW>());
        break;
    }
    assert(enemy && "Unknown enemy");
    if (boss)
        enemy->make_boss();
    enemy->path_id = path_id;
    auto& path = all_paths[path_id.start_node][path_id.path];
    enemy->position = glm::vec2(path[0]->x * 32 + 16, path[0]->y * 32 + 16);
    enemy->goal_path_node = 1;
    enemy->goal = sf::Vector2f(path[1]->x * 32 + 16, path[1]->y * 32 + 16);
    enemy->logic(0.0); // чтобы установить статус маскировки и goal.
    if (enemy->m_MREW_params_ptr != nullptr)
       MREW_enemies.push_back(enemy);
}

void EnemyManager::logic(double dtime) {
    for (auto it = m_enemies.begin(); it != m_enemies.end();) {
        IEnemy* enemy = *it;
        if (enemy->health <= 0) { /// удаляем уничтоженных.
            GameState::Instance().enemy_defeated(enemy->type); /// для статистики.
            m_destroyed_enemies.push_back(enemy->get_destroyed_enemy());
            GameState::Instance().player_coins_add(enemy->params.reward); /// награда.
            if (enemy->m_MREW_params_ptr != nullptr)
                MREW_enemies.remove(enemy);
            it = m_enemies.erase(it);
            continue;
        }
        enemy->logic(dtime); // логика врага
        if (enemy->path_is_completed) { //враг достиг конца маршрута, удаляем его.
            if (enemy->m_MREW_params_ptr != nullptr)
                MREW_enemies.remove(enemy);
            it = m_enemies.erase(it);
            continue;
        }
        ++it;
    }
    /// У уничтоженных врагов есть анимации уничтожения.
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
	
	for (auto enemy : m_enemies)
		enemy->draw(window);

}

void EnemyManager::draw_effects(sf::RenderWindow& window) {
    for (auto e : m_enemies)
        e->draw_effects(window);
    for (auto& smoke : m_smokes)
        smoke.draw(window);
    for (auto e : m_enemies)
        e->post_smoke_effects(window);
}

//Возвращает врага по id - O(1)!
IEnemy* EnemyManager::get_enemy_by_id(EnemyContainer::EnemyID id) {
    return m_enemies.get_enemy_by_id(id);
}
