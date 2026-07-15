#include "guns/radio_mast.h"
#include "net_manager.h"

RadioMast::RadioMast(int x_id, int y_id): IBuilding(x_id, y_id) {
    m_tower_sprite.setTexture(TextureManager::Instance().textures[TextureID::RadioMast]);
    m_tower_sprite.setOrigin(16, 16);
}

void RadioMast::draw(sf::RenderWindow& window) {
    m_tower_sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    window.draw(m_tower_sprite);
}

void RadioMast::draw_effects(sf::RenderWindow& window) {}

void RadioMast::logic(double dtime) {}

RadioMast::~RadioMast() {
    NetManager::Instance().radio_tower_deleted(x_id, y_id);
}
