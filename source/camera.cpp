#include "camera.h"
#include "glm/glm.hpp"
#include <iostream>

Camera::Camera(const sf::RenderWindow& window): m_window(window) {
	m_view = window.getDefaultView();
	m_base_view = m_view.getSize();
}

void Camera::apply(sf::RenderWindow& window) {
	window.setView(m_view);
}


bool Camera::process(const sf::Event& event) {
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Left) {
			m_is_captured = true;
			m_captured_view = m_view;
			m_captured_pos = m_window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y }, m_captured_view);
		}
		return true;
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Button::Left) {
			m_is_captured = false;
		}
		return true;
	}
	else if (event.type == sf::Event::MouseMoved) {
		if (m_is_captured) {
			sf::Vector2f current_pos = m_window.mapPixelToCoords({ event.mouseMove.x, event.mouseMove.y }, m_captured_view);
			sf::Vector2f move =  m_captured_pos - current_pos;
			auto center = m_captured_view.getCenter();
			m_view.setCenter(center.x + move.x, center.y + move.y);
		}
		return true;
	} 
	else if (event.type == sf::Event::MouseWheelScrolled) {
		m_scroll -= event.mouseWheelScroll.delta * 0.1;
		m_scroll = glm::clamp(m_scroll, 0.1f, 4.f);
		//m_view.zoom(1.0 - (event.mouseWheelScroll.delta) * 0.1);
		m_view.setSize(m_scroll * m_base_view);
		return true;
	}
	else if (event.type == sf::Event::Resized) {
		m_base_view = sf::Vector2f( event.size.width , event.size.height );
		m_view.setSize(m_scroll * m_base_view);
		return true;
	}
	return false;
}