#include "utils/sprite_chain.h"

void SpriteChain::draw(sf::RenderWindow& window) {
	if (!enabled)
		return;
	std::vector<Element> elements;
	get_elements(sf::Transform::Identity, elements);
	std::sort(elements.begin(), elements.end(), [](const Element& elem1, const Element& elem2) {return elem1.layer < elem2.layer; });
	for (auto& element : elements) {
		sf::RenderStates states;
		states.transform = element.transform;
        states.shader = element.shader;
		window.draw(*element.sprite, states);
	}
}


void SpriteChain::get_elements(const sf::Transform& parent_transform, std::vector<Element>& elements) {
	if (!enabled)
		return;
	sf::Transform transform = parent_transform;
	transform.rotate(m_rotation, m_position - m_position_origin + m_rotation_origin);
	transform.translate(m_position - m_position_origin);
	elements.push_back(Element{ &sprite,shader, layer, transform });
	for (auto& child : childs) {
		child.get_elements(transform, elements);
	}
}
