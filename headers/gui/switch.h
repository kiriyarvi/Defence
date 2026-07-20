#pragma once
#include "gui/widget.h"
#include <functional>

class Switch : public Widget, public Clickable {
public:
	Switch(bool initial_state, const std::function<void(bool)>& on_changed);
	Query on_event(EventContext context) override;
	void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
private:
	void set_state(bool state);
	bool m_state = false;
	sf::Sprite m_switch_sprite;
};

