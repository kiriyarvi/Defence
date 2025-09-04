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
    if (tile_id == 0b1000)
        sprite.setTextureRect(sf::IntRect(0, 0, 16, 16));
    else if (tile_id == 0b0100)
        sprite.setTextureRect(sf::IntRect(16, 0, 16, 16));
    else if (tile_id == 0b0010)
        sprite.setTextureRect(sf::IntRect(32, 0, 16, 16));
    else if (tile_id == 0b0001)
        sprite.setTextureRect(sf::IntRect(48, 0, 16, 16));
    else if (tile_id == 0b1100)
        sprite.setTextureRect(sf::IntRect(0, 16, 16, 16));
    else if (tile_id == 0b0110)
        sprite.setTextureRect(sf::IntRect(16, 16, 16, 16));
    else if (tile_id == 0b0011)
        sprite.setTextureRect(sf::IntRect(32, 16, 16, 16));
    else if (tile_id == 0b1001)
        sprite.setTextureRect(sf::IntRect(48, 16, 16, 16));
    else if (tile_id == 0b0111)
        sprite.setTextureRect(sf::IntRect(0, 32, 16, 16));
    else if (tile_id == 0b1011)
        sprite.setTextureRect(sf::IntRect(16, 32, 16, 16));
    else if (tile_id == 0b1101)
        sprite.setTextureRect(sf::IntRect(32, 32, 16, 16));
    else if (tile_id == 0b1110)
        sprite.setTextureRect(sf::IntRect(48, 32, 16, 16));
    else if (tile_id == 0b1111)
        sprite.setTextureRect(sf::IntRect(0, 48, 16, 16));
}

void Tile::draw(sf::RenderWindow& window, int x, int y) {
	sf::Sprite sprite;
	sprite.setPosition(x * 32, y * 32);
	sprite.setTexture(TextureManager::Instance().textures[TextureID::Grass]);
	window.draw(sprite);
 
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
    if (road_type == RoadType::Dirt)
        sprite.setTexture(TextureManager::Instance().textures[TextureID::RoadTileset]);
    else if (road_type == RoadType::Asphalt)
        sprite.setTexture(TextureManager::Instance().textures[TextureID::AsphaltRoadTileset]);
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

size_t mod(int a, int m) {
    return (a % m + m) % m;
}

void perlin_noise_octave(std::vector<std::vector<Tile>>& map, size_t S, size_t scale, double amplitude, std::vector<std::vector<glm::vec2>>& dir_map, sf::IntRect prohibited_zone, bool clear) {
    const size_t cells_count = S / scale;
    const size_t cell_size = scale;
    for (size_t x = 0; x < S; ++x) for (size_t y = 0; y < S; ++y) {
        if (prohibited_zone.contains(sf::Vector2i(x, y)))
            continue;
        if (clear)
            map.at(x).at(y).height = 0;
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

std::vector<std::vector<glm::vec2>> generate_directions(size_t N) {
    std::vector<std::vector<glm::vec2>> directions(N);
    for (size_t i = 0; i < N; ++i) {
        directions[i].resize(N);
        for (size_t j = 0; j < N; ++j)
            directions[i][j] = glm::circularRand(1.f);
    }
    return directions;
}

void TileMap::generate_dirt_road_height_map(sf::IntRect prohibited_zone) {
    size_t N = map.size();
    //1. Сгенерируем сетку с векторами в углах
    std::vector<std::vector<glm::vec2>> directions = generate_directions(N);
    //2. Окатвы шума Перлина
    int layers = log2(N);
    int scale = 2;
    double full_amp = 0;
    for (size_t i = 0; i < layers; ++i) {
        perlin_noise_octave(map, N, scale, scale, directions, prohibited_zone, i == 0);
        scale *= 2;
        full_amp += scale;
    }
    // нормировка.
    //m_test_image.create(map.size(), map.size());
    for (size_t i = 0; i < map.size(); ++ i)
        for (size_t j = 0; j < map.size(); ++j) {
            if (prohibited_zone.contains(sf::Vector2i(i, j)))
                continue;
            map[i][j].height = ((map[i][j].height + full_amp) / (2. * full_amp)); 
            int f = map[i][j].height * 255.f;
           // m_test_image.setPixel(i, j, sf::Color(f, f, f));
        }
}

void TileMap::generate_asphalt_road_height_map(sf::IntRect prohibited_zone) {
    size_t N = map.size();

    size_t S = 10;
    float d = 1. / S;
    for (size_t i = 0; i < S; ++i) {
        size_t x1 = rand() % N;
        size_t x2 = rand() % N;
        size_t y1 = rand() % N;
        size_t y2 = rand() % N;
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);
        for (size_t x = x1; x <= x2; ++x)  for (size_t y = y1; y <= y2; ++y) {
            if (prohibited_zone.contains(sf::Vector2i(x, y)))
                continue;
            if (i == 0)
                map[x][y].height = 0;
            map[x][y].height += d;
        }
    }


    // TODO Убрать.
    m_test_image.create(map.size(), map.size());
    for (size_t i = 0; i < map.size(); ++i)
        for (size_t j = 0; j < map.size(); ++j) {
            if (prohibited_zone.contains(sf::Vector2i(i, j)))
                continue;
            map[i][j].height = ((map[i][j].height + 1.) / (2.));
            int f = map[i][j].height * 255.f;
            m_test_image.setPixel(i, j, sf::Color(f, f, f));
        }
}

void TileMap::create_map(size_t N) {
    map.clear();
    map.resize(N);
    for (auto& r : map)
        r.resize(N);
    m_road_graph.end_nodes.clear();
    m_road_graph.start_nodes.clear();
    m_road_graph.nodes.clear();
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
    const glm::uvec2& goal,
    sf::IntRect zone,
    float direction_change_penalty = -1
) {
    int N = grid.size();
    std::priority_queue<Node, std::vector<Node>, std::greater<>> open_set;
    std::set<glm::uvec2, UVec2Comparator> closed_set;
    std::unordered_map<glm::uvec2, glm::uvec2> came_from;

    std::unordered_map<glm::uvec2, float> g_score;
    g_score[start] = 0.0;

    open_set.push({ start, heuristic(start, goal), 0.0, heuristic(start, goal) });

    std::vector<glm::ivec2> directions = {
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

        for (const glm::ivec2& dir : directions) {
            glm::ivec2 neighbor = { current.point.x + dir.x, current.point.y + dir.y };
            // Границы карты
            if (!zone.contains(sf::Vector2i(neighbor.x, neighbor.y)))
                continue;
            if (closed_set.count(neighbor))
                continue;
            if (grid[neighbor.x][neighbor.y].height >= 100)
                continue; // запрещенная клетка.
            auto& roads = grid[neighbor.x][neighbor.y].roads;
            int r = std::count(roads.begin(), roads.end(), true);

            glm::ivec2 prev_dir = glm::ivec2(current.point) - glm::ivec2(came_from[current.point]);
            float tentative_g = g_score[current.point] + grid[neighbor.x][neighbor.y].height + (r ? -1 : 0);
            if (direction_change_penalty > 0 && prev_dir != dir)
                tentative_g += direction_change_penalty;
            if (!g_score.count(neighbor) || tentative_g < g_score[neighbor]) {
                came_from[neighbor] = current.point;
                g_score[neighbor] = tentative_g;
                float h = heuristic(neighbor, goal);
                float f = tentative_g + h;
                open_set.push({ neighbor, f, tentative_g, h });
            }
        }
    }
    assert(false);
    // Путь не найден
    return {};
}


std::vector<glm::uvec2> generate_enters(
    size_t N,
    glm::uvec2 offset,
    size_t enters_gap,
    bool shuffle = true
) {
    int enters_count = N / enters_gap;
    std::vector<glm::uvec2> enters(enters_count);
    for (size_t i = 0; i < enters_count; ++i)
        enters[i] = offset + glm::uvec2{ 0,  std::min(rand() % enters_gap + i * enters_gap, N - 1) };
    if (shuffle) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(enters.begin(), enters.end(), gen);
    }
    return enters;
}


std::pair<std::vector<glm::uvec2>, std::vector<glm::uvec2>> generate_enters_and_exits(
    size_t N,
    glm::uvec2 offset,
    size_t enters_gap,
    size_t exits_gap
) {
    return std::make_pair(
        generate_enters(N, offset, enters_gap, false),
        generate_enters(N, offset + glm::uvec2{N - 1, 0}, enters_gap, true)
    );
}

void TileMap::generate_roads(const std::vector<glm::uvec2>& enters, const std::vector<glm::uvec2>& exits, std::vector<std::vector<glm::uvec2>>& paths, RoadType road_type, sf::IntRect zone, float road_cost,
    float direction_change_penalty) {
    size_t N = map.size();
    for (size_t i = 0; i < enters.size(); ++i) {
        size_t exit_index = i % exits.size();
        auto& path = paths.emplace_back(a_star(map, enters[i], exits[exit_index], zone, direction_change_penalty));
        map[enters[i].x][enters[i].y].roads[2] = true;
        map[exits[exit_index].x][exits[exit_index].y].roads[0] = true;
        if (road_cost >= 0)
            map[path[0].x][path[0].y].height = road_cost;
        if (map[path[0].x][path[0].y].road_type == RoadType::None)
            map[path[0].x][path[0].y].road_type = road_type;
        for (size_t i = 1; i < path.size(); ++i) {
            if (road_cost >= 0)
                map[path[i].x][path[i].y].height = road_cost;
            if (map[path[i].x][path[i].y].road_type == RoadType::None) {
                if (sf::IntRect(8,8,8,8).contains(sf::Vector2i(path[i].x, path[i].y)) && road_type == RoadType::Asphalt)
                    assert(false);
                map[path[i].x][path[i].y].road_type = road_type;
            }
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
    }
}



void TileMap::generate_map() {
    create_map(8); // выделяем память
    // 1. сгенерируем входы и выходы.
    size_t N = map.size();
    auto [enters, exits] = generate_enters_and_exits(N, glm::uvec2{ 0,0 }, 4, 4);
    // 2. генерируем дороги
    std::vector<std::vector<glm::uvec2>> paths;
    generate_dirt_road_height_map(sf::IntRect()); // генерируем карту высот
    generate_roads(enters, exits, paths, RoadType::Dirt, sf::IntRect(0,0,8,8));
    // 3. составляем road_graph
    create_road_graph();
    //m_test_texture.loadFromImage(m_test_image);
}


void TileMap::generate_by_enters_chain(const std::vector<std::vector<glm::uvec2>>& enters_chain, int N, int y, std::vector<std::vector<glm::uvec2>>& paths) {
    for (size_t i = 1; i < enters_chain.size(); ++i) {
        auto exits = enters_chain[i];
        for (auto& e : exits) e.x -= 1;
        generate_roads(enters_chain[i - 1], exits, paths, RoadType::Dirt, sf::IntRect((i - 1) * N, N * y, N, N));
    }
}

void TileMap::enlarge_map() {
    // 1. Выделяем память под расширенную карту и переносим в центр новой карты все старые постройки.
    size_t N = map.size();
    auto map_copy = std::move(map);
    map.clear();
    map.resize(3 * N);
    for (size_t i = 0; i < 3 * N; ++i)
        map[i].resize(3 * N);
    for (size_t x = 0; x < N; ++x) for (size_t y = 0; y < N; ++y) {
        map[N + x][N + y] = std::move(map_copy[x][y]);
        map[N + x][N + y].height = 200; // делаем клетки старой карты запретными.
    }
    //2. Генерируем асфальтированные дороги.
    auto [enters, exits] = generate_enters_and_exits(3 * N, glm::uvec2{ 0,0 }, 8, 8);
    std::vector<std::vector<glm::uvec2>> paths;
    generate_asphalt_road_height_map(sf::IntRect(N,N, N, N)); // генерируем карту высот для асфальтированных дорог
    generate_roads(enters, exits, paths, RoadType::Asphalt, sf::IntRect(0, 0, 3 * N, 3 * N), -1, 2);
    //3. Прокладываем грунтовые дороги
    generate_dirt_road_height_map(sf::IntRect(N, N, N, N)); // генерируем карту высот для грунтовых дорог.

    std::vector<std::vector<glm::uvec2>> enters_chain;

    //A. Генерируем верхний слой
    for (int x = 0; x < 4; ++x)
        enters_chain.push_back(generate_enters(N, {x * N, 0}, 4, x != 0));
    generate_by_enters_chain(enters_chain, N, 0, paths);
    //B. Генерируем центральный слой
    std::vector<glm::uvec2> nodes;
    for (auto& e : m_road_graph.start_nodes)
        nodes.push_back({ N - 1, N + e->y });
    generate_roads(generate_enters(N, { 0, N }, 4), nodes, paths, RoadType::Dirt, sf::IntRect(0, N, N, N));
    nodes.clear();
    for (auto& e : m_road_graph.end_nodes)
        nodes.push_back({ 2 * N, N + e->y });
    generate_roads(nodes, generate_enters(N, { 3 * N - 1, N }, 4), paths, RoadType::Dirt, sf::IntRect(2 * N, N, N, N));
    //C. Генерируем нижний слой
    enters_chain.clear();
    for (int x = 0; x < 4; ++x)
        enters_chain.push_back(generate_enters(N, { x * N, 2 * N }, 4, x != 0));
    generate_by_enters_chain(enters_chain, N, 2, paths);

    //3. Переделываем road_graph.
    create_road_graph();

    m_test_texture.loadFromImage(m_test_image);
}

void RoadGraph::clear() {
    nodes.clear();
    start_nodes.clear();
    end_nodes.clear();
}

void TileMap::create_road_graph() {
    m_road_graph.clear();
    size_t N = map.size();
    std::unordered_map<glm::uvec2, RoadGraph::Node*> pos_node;
    for (size_t x = 0; x < N; ++x)
        for (size_t y = 0; y < N; ++y)
            if (map[x][y].road_type != RoadType::None) {
                pos_node[glm::uvec2{ x, y }] = &m_road_graph.nodes.emplace_back(RoadGraph::Node( x, y ));
            }
    for (auto& n : m_road_graph.nodes) {
        auto& roads = map[n.x][n.y].roads;
        if (roads[0]) {
            if (n.x + 1 < N)
                n.relations.push_back(pos_node[{n.x + 1, n.y}]);
            else
                m_road_graph.end_nodes.push_back(&n);
        }
        if (roads[1] && n.y - 1 >= 0) {
            n.relations.push_back(pos_node[{n.x, n.y - 1}]);
        }
        if (roads[2]) {
            if (n.x - 1 >= 0)
                n.relations.push_back(pos_node[{n.x - 1, n.y}]);
            else
                m_road_graph.start_nodes.push_back(&n);
        }
        if (roads[3] && n.y + 1 < N) {
            n.relations.push_back(pos_node[{n.x, n.y + 1}]);
        }
    }
}

TileMap::TileMap() {
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

// -----------------------------
// Path hashing function
// -----------------------------
std::string RoadGraph::hash_path(const Path& path) const {
    std::string hash;
    for (const auto* node : path) {
        hash += std::to_string(node->x) + "," + std::to_string(node->y) + ";";
    }
    return hash;
}

// -----------------------------
// DFS to find random unique paths
// -----------------------------
void RoadGraph::dfs_random_paths(
    Node* current,
    Node* start_node,
    Path& current_path,
    std::unordered_set<Node*>& visited,
    Paths& result,
    int max_paths_per_node,
    int turns,
    int dx,
    std::mt19937& rng
) const {
    if (visited.count(current)) return;

    visited.insert(current);
    current_path.push_back(current);

    // Check if reached an end node
    if (std::find(end_nodes.begin(), end_nodes.end(), current) != end_nodes.end()) {
        result[start_node].push_back(current_path);
        current_path.pop_back();
        visited.erase(current);
        return;
    }
        
    std::vector<Node*> neighbors = current->relations;
    // Если уже достаточно много раз сворачивали назад - не позволяем свернуть назад еще раз, если есть выбор.
    if (neighbors.size() == 1) { // нет выбора
        int new_dx = neighbors[0]->x;
        if (new_dx < 0 && dx > 0) {
            ++turns;
            dx = -1;
        }
        if (turns <= 2)
            dfs_random_paths(neighbors[0], start_node, current_path, visited, result, max_paths_per_node, turns, dx, rng);
    }else {
        neighbors.erase(std::remove_if(neighbors.begin(), neighbors.end(), [&](Node* neight) {
            int dx = neight->x - current->x;
            return dx < 0;
        }), neighbors.end());
        std::shuffle(neighbors.begin(), neighbors.end(), rng);
        for (Node* neighbor : neighbors) {
            if (result[start_node].size() >= max_paths_per_node) break;
            int new_dx = neighbor->x;
            if (new_dx * dx == -1) {
                ++turns;
                dx = -dx;
            }
            if (turns <= 2)
                dfs_random_paths(neighbor, start_node, current_path, visited, result, max_paths_per_node, turns, dx, rng);
        }
    }
    current_path.pop_back();
    visited.erase(current);
}

// -----------------------------
// Main function
// -----------------------------
RoadGraph::Paths RoadGraph::find_all_paths() const {
    const int max_paths_per_node = 10;
    Paths result;
    std::random_device rd;
    std::mt19937 rng(rd());

    for (Node* start : start_nodes) {
        std::set<std::string> unique_paths;
        Path path;
        std::unordered_set<Node*> visited;

        dfs_random_paths(start, start, path, visited, result, max_paths_per_node, 0, 1, rng);
    }

    return result;
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

