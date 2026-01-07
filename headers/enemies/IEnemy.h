#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "glm/glm.hpp"
#include "params_manager.h"
#include "tile_map.h"

enum class EnemyType {
    Solder,
    Bike,
    Pickup,
    Truck,
    BTR,
    Tank,
    CruiserI,
    SmokeTruck
};

std::string to_string(EnemyType type);

class HealthIndicator {
public:
	void draw(sf::RenderWindow& window, float x, float y, float max_healf, float current_healf);
	float width = 16;
    sf::Color fill_color = sf::Color::Red;
};

class IDestroyedEnemy {
public:
	using Ptr = std::unique_ptr<IDestroyedEnemy>;
	virtual void draw(sf::RenderWindow& window) = 0;
	virtual void logic(double dtime_microseconds) = 0;
	virtual bool is_ready() = 0;
};

struct Collision {
    Collision(const glm::vec2& tl_vertex, const glm::vec2& br_vertex) : tl_vertex{ tl_vertex }, br_vertex{ br_vertex } {}
    Collision(const glm::vec2& box_r) : tl_vertex{ -box_r.x, -box_r.y }, br_vertex{ box_r.x, box_r.y } {}
    Collision(float rx, float ry) : tl_vertex{ -rx, -ry }, br_vertex{ rx, ry } {}
    glm::vec2 tl_vertex; // координаты левого верхнего угла относительно центра (position в IEnemy)
    glm::vec2 br_vertex;
};

class IEnemy {
public:
	using Ptr = std::unique_ptr<IEnemy>;
	IEnemy(const ParamsManager::Params::Enemies::Enemy& p, EnemyType t, Collision c);
	IEnemy(const IEnemy&) = delete;
	IEnemy& operator=(const IEnemy&) = delete;
	IEnemy(IEnemy&&) = default;
	IEnemy& operator=(IEnemy&&) = default;
	virtual ~IEnemy() = default;

	bool break_enemy(double repairing_time);

	virtual void draw(sf::RenderWindow& window) = 0;
	virtual void draw_effects(sf::RenderWindow& window);
    void post_smoke_effects(sf::RenderWindow& window);
	virtual bool logic(double dtime); // true --- достиг конца пути
	virtual glm::vec2 get_position() { return position; }
	virtual IDestroyedEnemy::Ptr get_destroyed_enemy() = 0;
    virtual void make_boss() { m_boss = true; }

    virtual void draw_collision(sf::RenderWindow& window);

	uint32_t id; // уникальный идентификатор врага
	const ParamsManager::Params::Enemies::Enemy& params;

	int health;
	glm::vec2 position = { 0,0 };
	float rotation = 0;

    RoadGraph::PathID path_id; // путь по которому движется враг.
	bool path_is_completed = false;
	
	bool infantry; // пехота (мотоциклист считается пехотой)
    EnemyType type;
	enum class Wheels {
		None,
		Wheels,
		Tracks,
        HeavyTracks
	} wheels;

    bool m_in_smoke = false; // находится ли враг внутри завесы.
    Collision collision;
    float m_bounding_box_border_scale = 0.5;
	sf::Vector2f goal; // текущая целевая точка
	int goal_path_node = 0; // номер узла в пути, к которому враг стремиться на данный момент
private:
	sf::Vector2i last_breaking_cell = { -1, -1 };
	bool repairing = false;
	double repairing_timer;
	double repairing_time;
	std::unique_ptr<sf::Sound> engine_sound;
    bool m_boss = false;
};
