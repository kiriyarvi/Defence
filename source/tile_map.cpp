#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"

#include "glm/glm.hpp"

#include <iostream>

void Tile::draw(sf::RenderWindow& window, int x, int y) {
	sf::Sprite sprite;
	sprite.setPosition(x * 32, y * 32);
	sprite.setTexture(TileMap::Instance().textures[TileTexture::Grass]);
	window.draw(sprite);

	int ends = std::count(roads.begin(), roads.end(), true);
	if (ends >= 2) {
		unsigned int tile_id = 0;
		for (int i = 0; i < 4; ++i) {
			tile_id *= 2;
			tile_id += roads[i];
		}
		sprite.setTexture(TileMap::Instance().textures[static_cast<TileTexture>(tile_id)]);
		window.draw(sprite);
	}
}

TileMap::TileMap() {
	textures[TileTexture::Grass].loadFromFile("tiles/grass.png");
	textures[TileTexture::Road1100].loadFromFile("tiles/road1100.png");
	textures[TileTexture::Road0110].loadFromFile("tiles/road0110.png");
	textures[TileTexture::Road0011].loadFromFile("tiles/road0011.png");
	textures[TileTexture::Road1001].loadFromFile("tiles/road1001.png");
	textures[TileTexture::Road1010].loadFromFile("tiles/road1010.png");
	textures[TileTexture::Road0101].loadFromFile("tiles/road0101.png");
	textures[TileTexture::Road0111].loadFromFile("tiles/road0111.png");
	textures[TileTexture::Road1011].loadFromFile("tiles/road1011.png");
	textures[TileTexture::Road1101].loadFromFile("tiles/road1101.png");
	textures[TileTexture::Road1110].loadFromFile("tiles/road1110.png");
	textures[TileTexture::Road1111].loadFromFile("tiles/road1111.png");

	textures[TileTexture::GunBase].loadFromFile("sprites/gun_base.png");
	textures[TileTexture::TwinGunTurret].loadFromFile("sprites/twin_gun_turret.png");
	textures[TileTexture::TwinGunUpperBarrel].loadFromFile("sprites/twin_gun_upper_barrel.png");
	textures[TileTexture::Shot].loadFromFile("sprites/shot.png");

	textures[TileTexture::AntitankGunTurret].loadFromFile("sprites/antitank_gun_turret.png");
	textures[TileTexture::AntitankGunBarrel].loadFromFile("sprites/antitank_gun_barrel.png");
	textures[TileTexture::AntitankGunTurretSubstrate].loadFromFile("sprites/antitank_gun_turret_substrate.png");
	//Test Map
	map[0][1].roads = { 1,0,1,0 };
	map[1][1].roads = { 1,0,1,0 };
	map[2][1].roads = { 0,0,1,1};
	map[2][2].roads = { 1,1,0,1 };
	map[3][2].roads = { 1,0,1,0 };
	map[4][2].roads = { 1,0,1,0 };
	map[5][2].roads = { 0,1,1,1 };
	map[5][1].roads = { 1,0,0,1 };
	map[6][1].roads = { 1,0,1,0 };
	map[7][1].roads = { 1,0,1,0 };

	map[2][3].roads = { 0,1,0,1 };
	map[2][4].roads = { 0,1,0,1 };
	map[2][5].roads = { 1,1,0,1 };
	map[2][6].roads = { 0,1,1,0 };
	map[1][6].roads = { 1,0,1,0 };
	map[0][6].roads = { 1,0,1,0 };

	map[3][5].roads = { 1,0,1,0 };
	map[4][5].roads = { 1,0,1,0 };
	map[5][5].roads = { 1,1,1,1 };

	map[5][6].roads = { 1,1,0,0 };
	map[6][6].roads = { 1,0,1,0 };
	map[7][6].roads = { 1,0,1,0 };

	map[6][5].roads = { 0,1,1,0 };
	map[6][4].roads = { 1,0,0,1 };
	map[7][4].roads = { 1,0,1,0 };

	map[5][1].roads = { 1,0,0,1 };
	map[6][1].roads = { 1,0,1,0 };
	map[7][1].roads = { 1,0,1,0 };

	map[5][4].roads = { 0,1,0,1 };
	map[5][3].roads = { 0,1,0,1 };
	
	m_road_graph.nodes.reserve(15);
	m_road_graph.nodes.push_back(RoadGraph::Node(0, 1)); auto& nodeA = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(2, 1)); auto& nodeB = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(2, 2)); auto& nodeC = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(2,5)); auto& nodeD = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(2, 6)); auto& nodeE = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(0, 6)); auto& nodeF = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(5, 2)); auto& nodeG = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(5, 1)); auto& nodeX = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(7, 1)); auto& nodeH = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(5, 5)); auto& nodeK = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(6, 5)); auto& nodeL = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(6, 4)); auto& nodeS = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(7, 4)); auto& nodeM = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(5, 6)); auto& nodeJ = m_road_graph.nodes.back();
	m_road_graph.nodes.push_back(RoadGraph::Node(7, 6)); auto& nodeN = m_road_graph.nodes.back();
	nodeA.relations.push_back(&nodeB);
	nodeB.relations.push_back(&nodeC);
	nodeC.relations.push_back(&nodeG);
	nodeC.relations.push_back(&nodeD);
	nodeD.relations.push_back(&nodeK);
	nodeE.relations.push_back(&nodeD);
	nodeF.relations.push_back(&nodeE);
	nodeG.relations.push_back(&nodeX);
	nodeG.relations.push_back(&nodeK);
	nodeX.relations.push_back(&nodeH);
	nodeK.relations.push_back(&nodeG);
	nodeK.relations.push_back(&nodeL);
	nodeK.relations.push_back(&nodeJ);
	nodeL.relations.push_back(&nodeS);
	nodeS.relations.push_back(&nodeM);
	nodeJ.relations.push_back(&nodeN);
	m_road_graph.start_nodes = { &nodeA, &nodeF };
	m_road_graph.end_nodes = { &nodeH, &nodeN, &nodeM };

}

void TileMap::build_guns() {
	map[3][1].building = std::make_unique<AntitankGun>();

	map[7][5].building = std::make_unique<TwinGun>();

	map[7][3].building = std::make_unique<TwinGun>();

	map[3][3].building = std::make_unique<TwinGun>();
	map[3][4].building = std::make_unique<TwinGun>();
	map[4][3].building = std::make_unique<TwinGun>();
	map[4][4].building = std::make_unique<TwinGun>();
}

void TileMap::draw(sf::RenderWindow& window) {
	for (int x = 0; x < map.size(); ++x)
		for (int y = 0; y < map[x].size(); ++y) {
			map[x][y].draw(window, x, y);
		}
	for (int x = 0; x < map.size(); ++x)
		for (int y = 0; y < map[x].size(); ++y) {
			if (map[x][y].building)
				map[x][y].building->draw(window, x, y);
		}
}

void TileMap::draw_effects(sf::RenderWindow& window) {
	for (int x = 0; x < map.size(); ++x)
		for (int y = 0; y < map[x].size(); ++y) {
			if (map[x][y].building)
				map[x][y].building->draw_effects(window, x, y);
		}
}

void TileMap::logic(double dtime) {
	for (int x = 0; x < map.size(); ++x)
		for (int y = 0; y < map[x].size(); ++y)
			if (map[x][y].building)
				map[x][y].building->logic(dtime, x, y);
}

std::vector<std::vector<RoadGraph::Node*>> RoadGraph::find_all_paths() const {
	std::vector<std::vector<Node*>> all_paths;

	for (Node* start : start_nodes) {
		std::vector<Node*> current_path;
		std::unordered_set<Node*> visited;
		dfs(start, current_path, visited, all_paths);
	}

	return all_paths;
}

void RoadGraph::dfs(Node* current,
	std::vector<Node*>& path,
	std::unordered_set<Node*>& visited,
	std::vector<std::vector<Node*>>& all_paths) const
{
	if (!current || visited.count(current)) return;

	path.push_back(current);
	visited.insert(current);

	// Если достигли одного из целевых узлов — сохраняем путь
	if (std::find(end_nodes.begin(), end_nodes.end(), current) != end_nodes.end()) {
		all_paths.push_back(path);
	}

	// Рекурсивно обходим соседей
	for (Node* neighbor : current->relations) {
		dfs(neighbor, path, visited, all_paths);
	}

	// Назад (backtracking)
	visited.erase(current);
	path.pop_back();
}

