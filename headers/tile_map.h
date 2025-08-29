#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
#include "texture_manager.h"

enum class BuildingType {
    Minigun,
    Spikes,
    Mine,
    Hedgehogs,
    AntitankGun,
    TwinGun
};

class MiniGun;
class Spikes;
class Hedgehog;
class AntitankGun;
class TwinGun;
class Mine;

class IBuildingVisitor {
public:
    virtual void visit(MiniGun& minigun) = 0;
    virtual void visit(Spikes& spikes) = 0;
    virtual void visit(Hedgehog& headgehogs) = 0;
    virtual void visit(AntitankGun& antitank_gun) = 0;
    virtual void visit(TwinGun& twingun) = 0;
    virtual void visit(Mine& mine) = 0;
};

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

class Tile {
public:
	TextureID background_texture = TextureID::Grass;
	std::array<bool, 4> roads; // Right, Up, Left, Down
	virtual void draw(sf::RenderWindow& window, int x, int y);
	virtual ~Tile() = default;

	Building building = nullptr;
};


class RoadGraph {
public:
	struct Node {
		Node(int x, int y) : x{ x }, y{ y } {}
		int x; int y;
		std::vector<Node*> relations;
	};
	std::vector<Node> nodes;
	std::vector<Node*> start_nodes;
	std::vector<Node*> end_nodes;

	// Найти все пути
    using Path = std::vector<Node*>;
    using Paths = std::map<Node*, std::vector<Path>>;
    struct PathID {
        RoadGraph::Node* start_node = nullptr;
        int path = 0;
    };

    Paths find_all_paths() const;
private:
	void dfs(Node* current,
		std::vector<Node*>& path,
		std::unordered_set<Node*>& visited,
		std::vector<std::vector<Node*>>& all_paths) const;
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
	std::array<std::array<Tile, 8>, 8> map;
	const RoadGraph& get_road_graph() { return m_road_graph; }
private:
	TileMap();
private:
	RoadGraph m_road_graph;
};
