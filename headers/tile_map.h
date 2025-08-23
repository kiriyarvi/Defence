#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

enum class TileTexture {
	Grass,
	Road1100 = 12,
	Road0110 = 6,
	Road0011 = 3,
	Road1001 = 9,
	Road1010 = 10,
	Road0101 = 5,
	Road0111 = 7,
	Road1011 = 11,
	Road1101 = 13,
	Road1110 = 14,
	Road1111 = 15,
	GunBase,
	TwinGunTurret,
	TwinGunUpperBarrel,
	Shot,
	AntitankGunTurret,
	AntitankGunBarrel,
	AntitankGunTurretSubstrate,
	MiniGun,
	MiniGunEquipment,
	Mine,
	MineBlast
};

class IBuilding {
public:
	virtual void draw(sf::RenderWindow& window, int x, int y) = 0;
	virtual void draw_effects(sf::RenderWindow& window, int x, int y) = 0;
	virtual void logic(double dtime, int x, int y) = 0;
	virtual bool is_destroyed() { return false; }
	virtual ~IBuilding() = default;
};

using Building = std::unique_ptr<IBuilding>;

class Tile {
public:
	TileTexture background_texture = TileTexture::Grass;
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
	std::vector<std::vector<Node*>> find_all_paths() const;
private:
	void dfs(Node* current,
		std::vector<Node*>& path,
		std::unordered_set<Node*>& visited,
		std::vector<std::vector<Node*>>& all_paths) const;
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
	std::unordered_map<TileTexture, sf::Texture> textures;
	const RoadGraph& get_road_graph() { return m_road_graph; }
	void build_guns();
private:
	TileMap();
	RoadGraph m_road_graph;
};