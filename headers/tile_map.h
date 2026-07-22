#pragma once

#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <random>
#include <glm/glm.hpp>
#include "texture_manager.h"
#include "guns/building.h"

enum class RoadType {
    None,
    Dirt,
    Asphalt
};

class TileMap;

class Tile {
public:
    Tile() = default;
    Tile(Tile&&) = default;
    Tile& operator=(Tile&&) = default;
	TextureID background_texture = TextureID::Grass;
	std::array<bool, 4> roads; // Right, Up, Left, Down
	void draw(TileMap& map, sf::RenderWindow& window, int x, int y);
	IBuilding* building = nullptr;
    float height = 0;
    RoadType road_type = RoadType::None;
};

// Может и можно убрать, ведь в Tile уже зашифрован этот граф
// и (x,y) по Tile можно определить, используя арифметику указателей,
// Но все же, если map не будет загружено в память полностью, будут проблемы.
// Поэтому стоить хранить логический класс путей.
// А если map так и не станет таким большим, чтобы стало необходимо
// выгружать часть ее на диск, можно будет убрать этот RoadGraph.
class RoadGraph {
public:
	struct Node {
		Node(int x, int y) : x{ x }, y{ y } {}
		int x; int y;
		std::vector<Node*> relations;
	};
	std::list<Node> nodes;
	std::list<Node*> start_nodes;
	std::list<Node*> end_nodes;

    void clear();

	// Найти все пути
    struct Path {
        Node* operator[](size_t i) const { return v[i]; }
        std::vector<Node*> v;
        float distance = 0.0;
        void calc_dist();
    };
    using Paths = std::map<Node*, std::vector<Path>>;
    struct PathID {
        RoadGraph::Node* start_node = nullptr;
        int path = 0;
    };

    Paths find_all_paths() const;
private:
    void dfs_random_paths(
        Node* current,
        Node* start_node,
        Path& current_path,
        std::unordered_set<Node*>& visited,
        Paths& result,
        int max_paths_per_node,
        int turns_back,
        int dx,
        std::mt19937& rng
    ) const;
};

class RouteDrawer {
public:
    RouteDrawer(const RoadGraph::Path& path);
    void draw(sf::RenderWindow& window);
    void logic(double dtime_mc);
private:
    sf::VertexArray m_vertex_array;
    float m_offset = 0;
};

class TileMap {
public:
    TileMap() = default;
	void draw(sf::RenderWindow& window);
	void draw_effects(sf::RenderWindow& window);
	void logic(double dtime);
    void generate_map();
    void enlarge_map();
    bool valid_ids(int x, int y);
	std::vector<std::vector<Tile>> map;
	const RoadGraph& get_road_graph() { return m_road_graph; }
    void create_tile_test_map();

    void delete_building(int x_id, int y_id);
    void add_building(std::unique_ptr<IBuilding>&& building);
private:
    void generate_dirt_road_height_map(sf::IntRect prohibited_zone);
    void generate_asphalt_road_height_map(sf::IntRect prohibited_zone);
    void generate_by_enters_chain(const std::vector<std::vector<glm::uvec2>>& enters_chain, int N, int y, std::vector<std::vector<glm::uvec2>>& paths);
    void create_map(size_t N);
    void generate_roads(
        const std::vector<glm::uvec2>& enters,
        const std::vector<glm::uvec2>& exits,
        std::vector<std::vector<glm::uvec2>>& paths,
        RoadType road_type,
        sf::IntRect zone,
        float road_cost = -1,
        float direction_change_penalty = -1
        );
    void create_road_graph();
private:
	RoadGraph m_road_graph;
    sf::Texture m_test_texture;
    sf::Image m_test_image;
    std::list<std::unique_ptr<IBuilding>> m_building_registry;
};
