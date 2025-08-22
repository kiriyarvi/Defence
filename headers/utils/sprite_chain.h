#pragma once
#include <SFML/Graphics.hpp>

#include <list>

// сначала позиционирование, потом вращение.
class SpriteChain {
public:
	SpriteChain() = default;
	SpriteChain(const sf::Sprite& s) : sprite(s) {}
	void set_position(float x, float y) { m_position = { x ,y }; }
	void set_position_origin(float x, float  y) { m_position_origin = { x, y }; }
	void set_rotation_origin(float x, float y) { m_rotation_origin = { x, y }; }
	void set_rotation(float rotation) { m_rotation = rotation; }

	void draw(sf::RenderWindow& window);
	int layer = 0;
	bool enabled = true;
	sf::Sprite sprite;
	std::list<SpriteChain> childs;
private:
	struct Element {
		sf::Sprite* sprite;
		int layer;
		sf::Transform transform;
	};
	void get_elements(const sf::Transform& parent_transform, std::vector<Element>& elements);
private:
	sf::Vector2f m_position_origin = {0,0};
	sf::Vector2f m_rotation_origin = {0,0};
	sf::Vector2f m_position = {0,0};
	float m_rotation = 0;
};

