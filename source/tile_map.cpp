#include "tile_map.h"
#include "enemy_manager.h"
#include "sound_manager.h"
#include "shader_manager.h"
#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "guns/minigun.h"
#include "guns/mine.h"

#include "glm/gtc/random.hpp"

#include "glm/glm.hpp"

#include <queue>
#include <iostream>
#include <random>

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

void get_road_sprite(const std::array<bool, 4>& int_nodes, sf::Sprite& sprite) {
    unsigned int tile_id = 0;
    for (int i = 0; i < 4; ++i) {
        tile_id *= 2;
        tile_id += int_nodes[i];
    }
    if (tile_id == 0b1001)
        sprite.setTextureRect(sf::IntRect(0, 0, 16, 16));
    else if (tile_id == 0b0011)
        sprite.setTextureRect(sf::IntRect(16, 0, 16, 16));
    else if (tile_id == 0b0110)
        sprite.setTextureRect(sf::IntRect(32, 0, 16, 16));
    else if (tile_id == 0b1100)
        sprite.setTextureRect(sf::IntRect(48, 0, 16, 16));
    else if (tile_id == 0b0100)
        sprite.setTextureRect(sf::IntRect(0, 16, 16, 16));
    else if (tile_id == 0b0010)
        sprite.setTextureRect(sf::IntRect(16, 16, 16, 16));
    else if (tile_id == 0b0001)
        sprite.setTextureRect(sf::IntRect(32, 16, 16, 16));
    else if (tile_id == 0b1000)
        sprite.setTextureRect(sf::IntRect(48, 16, 16, 16));
    else if (tile_id == 0b1111)
        sprite.setTextureRect(sf::IntRect(0, 32, 16, 16));
    else if (tile_id == 0b1110)
        sprite.setTextureRect(sf::IntRect(16, 32, 16, 16));
    else if (tile_id == 0b1101)
        sprite.setTextureRect(sf::IntRect(32, 32, 16, 16));
    else if (tile_id == 0b1011)
        sprite.setTextureRect(sf::IntRect(48, 32, 16, 16));
    else if (tile_id == 0b0111)
        sprite.setTextureRect(sf::IntRect(0, 48, 16, 16));
}

void Tile::draw(sf::RenderWindow& window, int x, int y) {
	sf::Sprite sprite;
	sprite.setPosition(x * 32, y * 32);
	sprite.setTexture(TextureManager::Instance().textures[TextureID::Grass]);
	window.draw(sprite);
    if (new_tiling_mode) {
        std::array<std::array<bool, 3>, 3> int_points;
        for (auto& row : int_points) for (auto& a : row) a = false;
        if (std::count(roads.begin(), roads.end(), true) == 0)
            return;
        int_points[1][1] = true;
        if (roads[0])
            int_points[2][1] = true;
        if (roads[1])
            int_points[1][0] = true;
        if (roads[2])
            int_points[0][1] = true;
        if (roads[3])
            int_points[1][2] = true;
        sprite.setTexture(TextureManager::Instance().textures[TextureID::RoadTileset]);
        std::array<bool, 4> top_left = { int_points[1][0], int_points[0][0], int_points[0][1], int_points[1][1] };
        std::array<bool, 4> bottom_left = { int_points[1][1], int_points[0][1], int_points[0][2], int_points[1][2] };
        std::array<bool, 4> top_right = { int_points[2][0], int_points[1][0], int_points[1][1], int_points[2][1] };
        std::array<bool, 4> bottom_right = { int_points[2][1], int_points[1][1], int_points[1][2], int_points[2][2] };
        sprite.setPosition(x * 32, y * 32);
        get_road_sprite(top_left, sprite);
        window.draw(sprite);

        get_road_sprite(bottom_left, sprite);
        sprite.setPosition(x * 32, y * 32 + 16);
        window.draw(sprite);

        get_road_sprite(top_right, sprite);
        sprite.setPosition(x * 32 + 16, y * 32);
        window.draw(sprite);

        get_road_sprite(bottom_right, sprite);
        sprite.setPosition(x * 32 + 16, y * 32 + 16);
        window.draw(sprite);
    }
    else {
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


}

size_t mod(int a, int m) {
    return (a % m + m) % m;
}

void perlin_noise_octave(std::vector<std::vector<Tile>>& map, size_t S, size_t scale, double amplitude, std::vector<std::vector<glm::vec2>>& dir_map) {
    const size_t cells_count = S / scale;
    const size_t cell_size = scale;
    for (size_t x = 0; x < S; ++x) for (size_t y = 0; y < S; ++y) {
        int i = cell_size * (x / cell_size);
        int j = cell_size * (y / cell_size);
        // обход по часовой.
        glm::uvec2 indicies[4] = {
            glm::uvec2{mod(i, S),mod(j, S)},
            glm::uvec2{mod(i + cell_size, S),mod(j, S)},
            glm::uvec2{mod(i + cell_size, S), mod(j + cell_size, S)},
            glm::uvec2{mod(i, S),mod(j + cell_size, S)}
        };
        glm::ivec2 indicies_raw[4] = {
            glm::ivec2{i,j},
            glm::ivec2{i + cell_size,j},
            glm::ivec2{i + cell_size, j + cell_size},
            glm::ivec2{i,j + cell_size}
        };
        double dots[4];
        for (size_t k = 0; k < 4; ++k) {
            auto& ind = indicies[k];
            auto& ind_raw = indicies_raw[k];
            //вычисляем вектор расстояния до углов (нормировка чтобы скалярное произведение было в [-1,1]).
            glm::vec2 d = glm::vec2{ x + 0.5 - ind_raw.x , y + 0.5 - ind_raw.y } * static_cast<float>(1 / (sqrt(2) * scale));
            dots[k] = glm::dot(dir_map[ind.x][ind.y], d);
        }
        assert(dots[0] <= 1.f && dots[1] <= 1.f && dots[2] <= 1.f && dots[3] <= 1.f);
        assert(dots[0] >= -1.f && dots[1] >= -1.f && dots[2] >= -1.f && dots[3] >= -1.f);
        //билинейная интерполяция
        // f(x,y) = b0 + b1 x + b2 y + b3 xy
        float b0 = dots[0];
        float b1 = dots[1] - b0;
        float b2 = dots[3] - b0;
        float b3 = dots[2] - b0 - b1 - b2;
        float cell_coord_x = (x % cell_size) / static_cast<float>(cell_size);
        float cell_coord_y = (y % cell_size) / static_cast<float>(cell_size);
        map.at(x).at(y).height += amplitude * (b0 + b1 * cell_coord_x + b2 * cell_coord_y + b3 * cell_coord_x * cell_coord_y);
    }
}

void TileMap::generate_height_map() {
    size_t N = map.size();
    //1. Сгенерируем сетку с векторами в углах
    std::vector<std::vector<glm::vec2>> directions(N);
    for (size_t i = 0; i < N; ++i) {
        directions[i].resize(N);
        for (size_t j = 0; j < N; ++j)
            directions[i][j] = glm::circularRand(1.f);
    }

    int layers = log2(N);
    int scale = 2;
    double full_amp = 0;
    for (size_t i = 0; i < layers; ++i) {
        perlin_noise_octave(map, N, scale, scale, directions);
        scale *= 2;
        full_amp += scale;
    }

    m_test_image.create(map.size(), map.size());
    for (size_t i = 0; i < map.size(); ++ i)
        for (size_t j = 0; j < map.size(); ++j) {
            map[i][j].height = ((map[i][j].height + full_amp) / (2. * full_amp)); // нормировка.
            int f = map[i][j].height * 255.f;
            m_test_image.setPixel(i, j, sf::Color(f, f, f));
        }
}

void TileMap::create_map(size_t N) {
    map.resize(N);
    for (auto& r : map)
        r.resize(N);
}

struct Node {
    glm::uvec2 point;
    float f, g, h;
    bool operator>(const Node& other) const {
        return f > other.f;
    }
};

// Эвристика — Манхэттенское расстояние
float heuristic(const glm::uvec2& a, const glm::uvec2& b) {
    return glm::abs(a.x - b.x) + glm::abs(a.y - b.y);
}

namespace std {
    template <>
    struct hash<glm::uvec2> {
        std::size_t operator()(const glm::uvec2& v) const noexcept {
            // Простой способ объединить хэши двух uints
            std::size_t h1 = std::hash<unsigned int>{}(v.x);
            std::size_t h2 = std::hash<unsigned int>{}(v.y);
            return h1 ^ (h2 << 1); // или std::hash-комбайнер получше
        }
    };
}

struct UVec2Comparator {
    bool operator()(const glm::uvec2& a, const glm::uvec2& b) const {
        return std::tie(a.x, a.y) < std::tie(b.x, b.y);
    }
};

std::vector<glm::uvec2> a_star(
    const std::vector<std::vector<Tile>>& grid,
    const glm::uvec2& start,
    const glm::uvec2& goal
) {
    int N = grid.size();
    std::priority_queue<Node, std::vector<Node>, std::greater<>> open_set;
    std::set<glm::uvec2, UVec2Comparator> closed_set;
    std::unordered_map<glm::uvec2, glm::uvec2> came_from;

    std::unordered_map<glm::uvec2, float> g_score;
    g_score[start] = 0.0;

    open_set.push({ start, heuristic(start, goal), 0.0, heuristic(start, goal) });

    std::vector<glm::uvec2> directions = {
        {0, 1}, {1, 0}, {0, -1}, {-1, 0}
    };

    while (!open_set.empty()) {
        Node current = open_set.top();
        open_set.pop();

        if (current.point == goal) {
            // Восстановление пути
            std::vector<glm::uvec2> path;
            glm::uvec2 p = goal;
            while (p != start) {
                path.push_back(p);
                p = came_from[p];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        closed_set.insert(current.point);

        for (const glm::uvec2& dir : directions) {
            glm::uvec2 neighbor = { current.point.x + dir.x, current.point.y + dir.y };
            // Границы карты
            if (neighbor.x < 0 || neighbor.y < 0 || neighbor.x >= N || neighbor.y >= N)
                continue;
            if (closed_set.count(neighbor))
                continue;

            auto& roads = grid[neighbor.x][neighbor.y].roads;
            int r = std::count(roads.begin(), roads.end(), true);
            float tentative_g = g_score[current.point] + grid[neighbor.x][neighbor.y].height + (r ? -1 : 0);

            if (!g_score.count(neighbor) || tentative_g < g_score[neighbor]) {
                came_from[neighbor] = current.point;
                g_score[neighbor] = tentative_g;
                float h = heuristic(neighbor, goal);
                float f = tentative_g + h;
                open_set.push({ neighbor, f, tentative_g, h });
            }
        }
    }

    // Путь не найден
    return {};
}


void TileMap::generate_roads() {
    // 1. сгенерируем входы и выходы.
    size_t N = map.size();
    // A. определим число входов и выходов.
    int enter_gap = 4;
    int exit_gap = 4;
    int enters_count = N / enter_gap;
    int exits_count = N / exit_gap;


    std::vector<size_t> enters(enters_count);
    std::vector<size_t> exits(exits_count);
    for (size_t i = 0; i < enters_count; ++i) {
        enters[i] = std::min(rand() % enter_gap + i * enter_gap, N - 1);
    }
    for (size_t i = 0; i < exits_count; ++i) {
        exits[i] = std::min(rand() % exit_gap + i * exit_gap, N - 1);
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(exits.begin(), exits.end(), gen);
    
    // 2. генерируем дороги
    std::vector<std::vector<glm::uvec2>> paths;
    generate_height_map(); // генерируем карту высот
    for (size_t i = 0; i < enters.size(); ++i) {
        size_t exit_index = i % exits_count;
        auto& path = paths.emplace_back(a_star(map, { 0, enters[i] }, { N - 1, exits[exit_index] }));
        // 3. выставляем тайлы
        map[0][enters[i]].roads[2] = true;
        map[N - 1][exits[exit_index]].roads[0] = true;
        for (size_t i = 1; i < path.size(); ++i) {
            auto p = path[i];
            auto p_prev = path[i - 1];
            glm::ivec2 dir{ p.x - (int)p_prev.x, p.y - (int)p_prev.y };
            if (dir.x == 1 && dir.y == 0) {
                map[p.x][p.y].roads[2] = true;
                map[p_prev.x][p_prev.y].roads[0] = true;
            }
            else if (dir.x == 0 && dir.y == 1) {
                map[p.x][p.y].roads[1] = true;
                map[p_prev.x][p_prev.y].roads[3] = true;
            }
            else if (dir.x == -1 && dir.y == 0) {
                map[p.x][p.y].roads[0] = true;
                map[p_prev.x][p_prev.y].roads[2] = true;
            }
            else if (dir.x == 0 && dir.y == -1) {
                map[p.x][p.y].roads[3] = true;
                map[p_prev.x][p_prev.y].roads[1] = true;
            }
        }
        for (auto& p : path)
            m_test_image.setPixel(p.x, p.y, sf::Color(i * 100, i * 100, i * 100));
    }
    // 4. составляем road_graph
    for (size_t i = 0; i < paths.size(); ++i) {
        auto& path = paths[i];
        RoadGraph::Node* prev_node = nullptr;
        RoadGraph::Node* last_node = nullptr;
        for (auto& node : path) {
            auto it = std::find_if(m_road_graph.nodes.begin(), m_road_graph.nodes.end(), [&](const RoadGraph::Node& rg_node) {
                return rg_node.x == node.x && rg_node.y == node.y;
            });
            if (it == m_road_graph.nodes.end()) {
                last_node = &m_road_graph.nodes.emplace_back(RoadGraph::Node(node.x, node.y));
                if (prev_node)
                    prev_node->relations.push_back(last_node);
                else // это start node
                    m_road_graph.start_nodes.push_back(last_node);
            }
            else {
                last_node = &*it;
                if (prev_node) {
                    if (std::find(prev_node->relations.begin(), prev_node->relations.end(), last_node) == prev_node->relations.end())
                        prev_node->relations.push_back(last_node);
                }
                else // это start node
                    m_road_graph.start_nodes.push_back(last_node);
            }
            prev_node = last_node;
        }
        m_road_graph.end_nodes.push_back(last_node); // prev_node == last_node
    }

    /*for (auto& n: m_road_graph.nodes) {
        for (auto r : n.relations) {
            std::cout << "(" << n.x << ", " << n.y << ") -> (" << r->x << ", " << r->y << ")" << std::endl;
        }
    }*/
}

TileMap::TileMap() {
    create_map(8); // выделяем память
    generate_roads(); // генерируем дороги

    m_test_texture.loadFromImage(m_test_image);
	//Test Map
	/*map[0][1].roads = { 1,0,1,0 };
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
	map[5][3].roads = { 0,1,0,1 };*/
	
	/*m_road_graph.nodes.reserve(15);
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
	m_road_graph.end_nodes = { &nodeH, &nodeN, &nodeM };*/
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

    sf::Sprite sp(m_test_texture);
    sp.setOrigin(sp.getTextureRect().width, sp.getTextureRect().height);
    window.draw(sp);

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
    shader.setUniform("offset_y", 0.f);
    states.shader = &shader;
    window.draw(m_vertex_array, states);
}

void RouteDrawer::logic(double dtime_mc) {
    m_offset += 0.5 * dtime_mc;
    if (m_offset > 10 * 1000 * 1000) {
        m_offset -= 10 * 1000 * 1000;
    }
}

