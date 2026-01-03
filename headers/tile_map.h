#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <random>
#include <glm/glm.hpp>
#include "texture_manager.h"

enum class BuildingType {
    Minigun,
    Spikes,
    Mine,
    Hedgehogs,
    AntitankGun,
    TwinGun,
    Radar
};

class MiniGun;
class Spikes;
class Hedgehog;
class AntitankGun;
class TwinGun;
class Mine;
class Radar;

class IBuildingVisitor {
public:
    virtual void visit(MiniGun& minigun) = 0;
    virtual void visit(Spikes& spikes) = 0;
    virtual void visit(Hedgehog& headgehogs) = 0;
    virtual void visit(AntitankGun& antitank_gun) = 0;
    virtual void visit(TwinGun& twingun) = 0;
    virtual void visit(Mine& mine) = 0;
    virtual void visit(Radar& mine) = 0;
};

std::string to_string(BuildingType type);


#define ACCEPT(Type) \
void accept(IBuildingVisitor& visitor) override { visitor.visit(*this); }

class IBuilding {
public:
	virtual void draw(sf::RenderWindow& window, int x_id, int y_id) = 0;
	virtual void draw_effects(sf::RenderWindow& window, int x_id, int y_id) = 0;
	virtual void logic(double dtime, int x_id, int y_id) = 0;
	virtual bool is_destroyed() { return false; }
    virtual void accept(IBuildingVisitor& visitor) = 0;
	virtual ~IBuilding() = default;
};


using Building = std::unique_ptr<IBuilding>;

enum class RoadType {
    None,
    Dirt,
    Asphalt
};

class Tile {
public:
    Tile() = default;
    Tile(Tile&&) = default;
    Tile& operator=(Tile&&) = default;
	TextureID background_texture = TextureID::Grass;
	std::array<bool, 4> roads; // Right, Up, Left, Down
	virtual void draw(sf::RenderWindow& window, int x, int y);
	virtual ~Tile() = default;

	Building building = nullptr;
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
    using Path = std::vector<Node*>;
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

    std::string hash_path(const Path& path) const;
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
	// Получение единственного экземпляра
	static TileMap& Instance() {
		static TileMap instance; // Создаётся при первом вызове, потокобезопасно в C++11+
		return instance;
	}

	// Удаляем копирование и перемещение
	TileMap(const TileMap&) = delete;
	TileMap& operator=(const TileMap&) = delete;
	TileMap(TileMap&&) = delete;
	TileMap& operator=(TileMap&&) = delete;
	void draw(sf::RenderWindow& window);
	void draw_effects(sf::RenderWindow& window);
	void logic(double dtime);
    void generate_map();
    void enlarge_map();
    bool valid_ids(int x, int y);
	std::vector<std::vector<Tile>> map;
	const RoadGraph& get_road_graph() { return m_road_graph; }
    void create_tile_test_map();
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
	TileMap();
private:
	RoadGraph m_road_graph;
    sf::Texture m_test_texture;
    sf::Image m_test_image;
};
