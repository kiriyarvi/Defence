#include <guns/radio_tower.h>


RadioTower::RadioTower(int x_id, int y_id): IBuilding(x_id, y_id) {
    m_tower_sprite.setTexture(TextureManager::Instance().textures[TextureID::RadioTower]);
    m_tower_sprite.setOrigin(16, 16);
}

void RadioTower::draw(sf::RenderWindow& window) {
    m_tower_sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    window.draw(m_tower_sprite);
}

void RadioTower::draw_effects(sf::RenderWindow& window) {}

void RadioTower::logic(double dtime) {}
