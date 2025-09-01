#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "shader_manager.h"
#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "guns/minigun.h"
#include "guns/mine.h"


#include "glm/glm.hpp"

#include <iostream>

std::string to_string(BuildingType type) {
    std::unordered_map<BuildingType, std::string> m{
        {BuildingType::AntitankGun, "Противотанковая пушка"},
        {BuildingType::Hedgehogs, "Противотанковые ежи"},
        {BuildingType::Mine, "Мина"},
        {BuildingType::Minigun, "Пулемет"},
        {BuildingType::Spikes, "Шипы"},
        {BuildingType::TwinGun, "Сдвоенная пушка"}
    };
    return m[type];
}

void Tile::draw(sf::RenderWindow& window, int x, int y) {
	sf::Sprite sprite;
	sprite.setPosition(x * 32, y * 32);
	sprite.setTexture(TextureManager::Instance().textures[TextureID::Grass]);
	window.draw(sprite);

	int ends = std::count(roads.begin(), roads.end(), true);
	if (ends >= 2) {
		unsigned int tile_id = 0;
		for (int i = 0; i < 4; ++i) {
			tile_id *= 2;
			tile_id += roads[i];
		}
		sprite.setTexture(TextureManager::Instance().textures[static_cast<TextureID>(tile_id)]);
		window.draw(sprite);
	}
}



TileMap::TileMap() {
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
		for (int y = 0; y < map[x].size(); ++y) {
			if (map[x][y].building) {
				map[x][y].building->logic(dtime, x, y);
				if (map[x][y].building->is_destroyed())
					map[x][y].building.reset();
			}
		}
}

RoadGraph::Paths RoadGraph::find_all_paths() const {
    Paths all_paths;

	for (Node* start : start_nodes) {
		std::vector<Node*> current_path;
		std::unordered_set<Node*> visited;
        dfs(start, current_path, visited, all_paths[start]);
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


RouteDrawer::RouteDrawer(const RoadGraph::Path& path) {
    int N = path.size();
    m_vertex_array.resize(N * 2);
    glm::vec2 begin = { path[0]->x * 32 + 16, path[0]->y * 32 + 16 };
    glm::vec2 next_begin = { path[1]->x * 32 + 16, path[1]->y * 32 + 16 };
    glm::vec2 end = { path.back()->x * 32 + 16, path.back()->y * 32 + 16 };
    glm::vec2 prev_end = { path[N - 2]->x * 32 + 16, path[N - 2]->y * 32 + 16 };
    glm::vec2 dir_begin = glm::normalize(next_begin - begin);
    dir_begin = glm::vec2{ -dir_begin.y, dir_begin.x };
    glm::vec2 dir_end = glm::normalize(end - prev_end);
    dir_end = glm::vec2{ -dir_end.y, dir_end.x };

    float width = 4;


    m_vertex_array[0].position.x = begin.x +  width * dir_begin.x;
    m_vertex_array[0].position.y = begin.y +  width * dir_begin.y;
    m_vertex_array[1].position.x = begin.x -  width * dir_begin.x;
    m_vertex_array[1].position.y = begin.y -  width * dir_begin.y;

    m_vertex_array[2 * (N - 1)].position.x = end.x + width * dir_end.x;
    m_vertex_array[2 * (N - 1)].position.y = end.y + width * dir_end.y;
    m_vertex_array[2 * (N - 1) + 1].position.x = end.x - width * dir_end.x;
    m_vertex_array[2 * (N - 1) + 1].position.y = end.y - width * dir_end.y;

    for (size_t i = 1; i < N - 1; ++i) {
        glm::vec2 prev = { path[i - 1]->x * 32 + 16, path[i - 1]->y * 32 + 16 };
        glm::vec2 curr = { path[i]->x * 32 + 16, path[i]->y * 32 + 16 };
        glm::vec2 next = { path[i + 1]->x * 32 + 16, path[i + 1]->y * 32 + 16 };
        glm::vec2 A = glm::normalize(next - curr);
        glm::vec2 B = glm::normalize(prev - curr);
        glm::vec2 dir = glm::normalize(A + B);
        if (glm::length(A + B) > 0) {
            if (dir.x * A.y - dir.y * A.x > 0) // изменим знак, если dir смотрит не в ту сторону
                dir = -dir;
            float cos = glm::abs(glm::dot(dir, A));
            m_vertex_array[2 * i].position.x = curr.x + width * dir.x / cos;
            m_vertex_array[2 * i].position.y = curr.y + width * dir.y / cos;
            m_vertex_array[2 * i + 1].position.x = curr.x - width * dir.x / cos;
            m_vertex_array[2 * i + 1].position.y = curr.y - width * dir.y / cos;
        }
        else {
            dir = glm::normalize(next - prev);
            dir = glm::vec2{ -dir.y, dir.x };
            m_vertex_array[2 * i].position.x = curr.x + width * dir.x;
            m_vertex_array[2 * i].position.y = curr.y + width * dir.y;
            m_vertex_array[2 * i + 1].position.x = curr.x - width * dir.x;
            m_vertex_array[2 * i + 1].position.y = curr.y - width * dir.y;
        }
    }

    for (size_t i = 0; i < 2 * N; ++i) {
        m_vertex_array[i].texCoords = m_vertex_array[i].position;
    }

    m_vertex_array.setPrimitiveType(sf::TrianglesStrip);
}

void RouteDrawer::draw(sf::RenderWindow& window) {
    sf::RenderStates states;
    states.texture = &TextureManager::Instance().textures[TextureID::Path];
    auto& shader = ShaderManager::Instance().shaders[Shader::Scroll];
    shader.setUniform("texture", sf::Shader::CurrentTexture);
    shader.setUniform("offset_x", -(float)m_offset / (1000 * 1000));
    states.shader = &shader;
    window.draw(m_vertex_array, states);
}

void RouteDrawer::logic(double dtime_mc) {
    m_offset += 0.5 * dtime_mc;
    if (m_offset > 10 * 1000 * 1000) {
        m_offset -= 10 * 1000 * 1000;
    }
}

