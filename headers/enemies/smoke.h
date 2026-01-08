#pragma once
#include "glm/gtc/random.hpp"
#include <glm/vec2.hpp>
#include <SFML/Graphics.hpp>
#include <utils/animation.h>

class Smoke {
public:
    Smoke(const glm::vec2& pos, float r, float duration);
    bool logic(double dtime);
    void draw(sf::RenderWindow& window);
    bool active() const; /// true = smoke может скрывать врагов.
    glm::vec2 m_pos;
    float m_max_radius;
private:
    Animation m_animation;
    float m_duration;

    bool m_enabled = false;
    size_t m_max_particles = 70;

    sf::Sprite m_particle_sprite;



    float m_global_timer = 0.0;


    float m_particle_produce_duration = 0.0;
    float m_particle_flow_duration = 0.0;
    float m_particle_fade_duration = 0.0;
    float m_activation_time = 0.0;
    float m_deactivation_time = 0.0;

    struct Particle {
        Particle(const glm::vec2& vel) : m_pos{ 0,0 } {
            m_vel = vel;
            texture_id = rand() % 16;
            m_tor_vel = glm::linearRand(-1.0, 1.0);
        }
        glm::vec2 m_pos;
        glm::vec2 m_vel; //собственная скорость частицы, суммируется с полем.
        float m_scale = 0.0;
        int texture_id = 0;
        float m_timer = 0.0;
        float m_fade = 1.0;
        float m_tor_vel = 0.0;
        float m_rot = 0.0;
    };
    std::vector<Particle> m_particles;
};
