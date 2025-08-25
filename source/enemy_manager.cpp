#include "enemy_manager.h"
#include "sound_manager.h"
#include "game_state.h"
#include <glm/glm.hpp>

#include "enemies/simple_enemy.h"
#include "enemies/solder.h"

EnemyManager::EnemyManager() {
	all_paths = TileMap::Instance().get_road_graph().find_all_paths();
	enemy_textures[EnemyTexturesID::Tank].loadFromFile("sprites/tank.png");
	enemy_textures[EnemyTexturesID::TankDestroyed].loadFromFile("sprites/tank_destroyed.png");
	enemy_textures[EnemyTexturesID::Truck].loadFromFile("sprites/truck.png");
	enemy_textures[EnemyTexturesID::TruckDestroyed].loadFromFile("sprites/truck_destroyed.png");
	enemy_textures[EnemyTexturesID::MedBlustOfDestruction1].loadFromFile("sprites/med_blust_of_destruction1.png");
	enemy_textures[EnemyTexturesID::MedBlustOfDestruction2].loadFromFile("sprites/med_blust_of_destruction2.png");
	enemy_textures[EnemyTexturesID::Bike].loadFromFile("sprites/bike.png");
	enemy_textures[EnemyTexturesID::SolderWalkAnimation].loadFromFile("sprites/solder_walk_animation.png");
	enemy_textures[EnemyTexturesID::SolderAmmunition].loadFromFile("sprites/solder_ammunition.png");
	enemy_textures[EnemyTexturesID::DeadSolder].loadFromFile("sprites/dead_solder.png");
	enemy_textures[EnemyTexturesID::DestroyedBike].loadFromFile("sprites/destroyed_bike.png");
	enemy_textures[EnemyTexturesID::DoubleBlust].loadFromFile("sprites/double_blust.png");
	enemy_textures[EnemyTexturesID::Blusts16x16].loadFromFile("sprites/16x16_blusts.png");
	enemy_textures[EnemyTexturesID::RepairWrench].loadFromFile("sprites/repair_wrench.png");
}

void EnemyManager::spawn() {
	int enemy = rand() % 4;
	if (enemy == 0)
		m_enemies.push_back(std::make_unique<Solder>());
	else if (enemy == 1)
		m_enemies.push_back(std::make_unique<Truck>());
	else if (enemy == 2)
		m_enemies.push_back(std::make_unique<Tank>());
	else 
		m_enemies.push_back(std::make_unique<Bike>());
	m_enemies.back()->path_id = rand() % all_paths.size();
	m_enemies.back()->id = ++current_max_id;
	m_enemies.back()->logic(0.0); // чтобы установить верную позицию.
	if (current_max_id > 32768)
		current_max_id = 0;
}

void EnemyManager::logic(double dtime) {
	std::vector<IEnemy::Ptr> new_enemies;
	for (auto& enemy : m_enemies) {
		if (enemy->health <= 0) {
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
}


void EnemyManager::draw(sf::RenderWindow& window) {
	for (auto& destroyed_enemy : m_destroyed_enemies) {
		destroyed_enemy->draw(window);
	}
	
	for (auto& enemy : m_enemies)
		enemy->draw(window);

}

IEnemy* EnemyManager::get_enemy_by_id(uint32_t id) {
	auto it = std::find_if(m_enemies.begin(), m_enemies.end(), [id](const IEnemy::Ptr& enemy) {return enemy->id == id; });
	return it == m_enemies.end() ? nullptr : it->get();
}
