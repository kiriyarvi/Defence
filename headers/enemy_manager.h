#pragma once
#include "tile_map.h"
#include "SFML/Audio.hpp"
#include "enemies/IEnemy.h"

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
	DestroyedBike
};


class EnemyManager {
public:
	static EnemyManager& Instance() {
		static EnemyManager instance; // �������� ��� ������ ������, ��������������� � C++11+
		return instance;
	}

	void spawn();

	// ������� ����������� � �����������
	EnemyManager(const EnemyManager&) = delete;
	EnemyManager& operator=(const EnemyManager&) = delete;
	EnemyManager(EnemyManager&&) = delete;
	EnemyManager& operator=(EnemyManager&&) = delete;
	void logic(double dtime);
	void draw(sf::RenderWindow& window);
	IEnemy* get_enemy_by_id(uint32_t id);
	std::vector<std::vector<RoadGraph::Node*>> all_paths;
	std::unordered_map<EnemyTexturesID, sf::Texture> enemy_textures;
	std::vector<IEnemy::Ptr> m_enemies;
private:
	EnemyManager();
	uint32_t current_max_id = 0;
	std::list<DestroyedEnemy> m_destroyed_enemies;
};