#pragma once
#include "gui/widget.h"
#include <functional>

class Switch :public HoverableClickableWidget {
public:
	Switch(bool initial_state, const std::function<void(bool)>& on_changed);
	void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
private:
	void set_state(bool state);
	bool m_state = false;
	sf::Sprite m_switch_sprite;
};

