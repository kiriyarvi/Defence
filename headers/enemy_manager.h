#pragma once
#include "tile_map.h"
#include "SFML/Audio.hpp"
#include "enemies/IEnemy.h"
#include "wave_controller.h"

enum class EnemyTexturesID {
	Tank,
	TankDestroyed,
	Truck,
	TruckDestroyed,
	MedBlustOfDestruction1,
	MedBlustOfDestruction2,
	Bike,
	SolderWalkAnimation,
	SolderAmmunition,
	DeadSolder,
	DestroyedBike,
	DoubleBlust,
	Blusts16x16,
	RepairWrench,
    Pickup,
    PickupDestroyed,
    BTR,
    BTRDestroyed,
    Trucks,
    CruiserIBase,
    CruiserIEquipment
};


class EnemyManager {
public:
	static EnemyManager& Instance() {
		static EnemyManager instance; // Создаётся при первом вызове, потокобезопасно в C++11+
		return instance;
	}

	void spawn(EnemyType type, int path);

	// Удаляем копирование и перемещение
	EnemyManager(const EnemyManager&) = delete;
	EnemyManager& operator=(const EnemyManager&) = delete;
	EnemyManager(EnemyManager&&) = delete;
	EnemyManager& operator=(EnemyManager&&) = delete;
	void logic(double dtime); // если возвращает true --- спавнеры кончились.
	void draw(sf::RenderWindow& window);
	IEnemy* get_enemy_by_id(uint32_t id);
	std::vector<std::vector<RoadGraph::Node*>> all_paths;
	std::unordered_map<EnemyTexturesID, sf::Texture> enemy_textures;
	std::vector<IEnemy::Ptr> m_enemies;
    void init();
private:
	EnemyManager();
	uint32_t current_max_id = 0;
	std::list<IDestroyedEnemy::Ptr> m_destroyed_enemies;
    std::unique_ptr<WaveController> m_wave_controller;
};
