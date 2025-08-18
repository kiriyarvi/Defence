#pragma once
#include <SFML/Graphics.hpp>

class Camera {
public:
	Camera(const sf::RenderWindow& window);
	void apply(sf::RenderWindow& window);
	bool process(const sf::Event& event);
private:
	sf::View m_view;
	const sf::RenderWindow& m_window;
	sf::Vector2f m_base_view = { 200,200 };
	float m_scroll = 1.0;
	sf::Vector2f m_captured_pos = { 0,0 };
	sf::View m_captured_view;
	bool m_is_captured = false;
};


