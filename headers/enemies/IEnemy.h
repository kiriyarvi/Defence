#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "glm/glm.hpp"



class HealthIndicator {
public:
	void draw(sf::RenderWindow& window, float x, float y, float max_healf, float current_healf);
	float width = 16;
};

class DestroyedEnemy {
public:
	sf::Sprite sprite;
	void draw(sf::RenderWindow& window);
	void logic(double dtime_microseconds);
	bool is_ready();
private:
	float compute_k();
	double animation_time = 0;
	double animation_duration = 2;
};

class IEnemy {
public:
	using Ptr = std::unique_ptr<IEnemy>;
	IEnemy() = default;
	IEnemy(const IEnemy&) = delete;
	IEnemy& operator=(const IEnemy&) = delete;
	IEnemy(IEnemy&&) = default;
	IEnemy& operator=(IEnemy&&) = default;
	virtual ~IEnemy() = default;

	virtual void draw(sf::RenderWindow& window) = 0;
	virtual bool logic(double dtime); // true --- ������ ����� ����
	virtual glm::vec2 get_position() { return position; }
	virtual DestroyedEnemy get_destroyed_enemy() = 0;
	uint32_t id; // ���������� ������������� �����
	float speed;
	int health;
	int full_health;
	glm::vec2 position = { 0,0 };
	float rotation = 0;

	int path_id = 0; // ���� �� �������� �������� ����.
	bool path_is_completed = false;
private:
	sf::Vector2f goal; // ������� ������� �����
	int goal_path_node = 0; // ����� ���� � ����, � �������� ���� ���������� �� ������ ������
	std::unique_ptr<sf::Sound> engine_sound;
};