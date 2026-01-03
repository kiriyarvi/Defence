#include "guns/radar.h"


Radar::Radar(): m_params(ParamsManager::Instance().params.guns.radar) {
    m_radar_sprite.setTexture(TextureManager::Instance().textures[TextureID::Radar]);
    m_radar_sprite.setOrigin(16, 16);

    m_base_sprite.setTexture(TextureManager::Instance().textures[TextureID::GunBase]);
    m_base_sprite.setOrigin(16, 16);
}

void Radar::draw(sf::RenderWindow& window, int x_id, int y_id) {
    m_base_sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    window.draw(m_base_sprite);
    m_radar_sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    m_radar_sprite.setRotation(m_rotation);
    window.draw(m_radar_sprite);
}

void Radar::draw_effects(sf::RenderWindow& window, int x_id, int y_id) {
    sf::CircleShape circ(m_params.radius * 32);
    circ.setOrigin(m_params.radius * 32, m_params.radius * 32);
    circ.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    circ.setFillColor(sf::Color::Transparent);
    circ.setOutlineThickness(1);
    circ.setOutlineColor(sf::Color::Green);
    window.draw(circ);
}



void Radar::logic(double dtime_microseconds, int x_id, int y_id) {
    m_rotation += dtime_microseconds / (1000.f * 1000.f) * 20.f;
}
