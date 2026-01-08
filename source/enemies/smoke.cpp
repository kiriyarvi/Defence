#include <enemies/smoke.h>
#include <texture_manager.h>
#include <resource_manager.h>
#include <glm/gtx/compatibility.hpp>

Smoke::Smoke(const glm::vec2& pos, float r, float duration) :
    m_pos{ pos }, m_max_radius{ r * 32 }, m_duration{ duration }, m_animation(duration)
{
    m_particle_sprite.setTexture(TextureManager::Instance().textures[TextureID::SmokeParticles]);
    m_particle_produce_duration = 0.2f * m_duration * 1000 * 1000;
    m_particle_flow_duration = 0.5f * m_duration * 1000 * 1000;
    m_particle_fade_duration = 0.3f * m_duration * 1000 * 1000;
    m_activation_time = 0.1f * m_duration * 1000 * 1000;
    m_deactivation_time = m_duration * 1000 * 1000;
    m_enabled = true;
}

float smoothstep(float x) {
    return x * x * (3 - 2 * x);
}

bool Smoke::logic(double dtime) {
    m_global_timer += dtime;
    // +m_particle_produce_duration чтобы последняя частица успела испариться.
    if (m_global_timer >= m_duration * 1000.f * 1000.f + m_particle_produce_duration)
        return false;
    float dp = dtime / m_particle_produce_duration;
    for (auto& particle : m_particles) {
        particle.m_timer += dtime;
        float curl_fade = 1.;
        if (particle.m_timer <= m_particle_produce_duration) {
            // учет импульса, данного при рождении частицы.
            float p = particle.m_timer / m_particle_produce_duration;
            particle.m_pos += particle.m_vel * dp * (1 - p) * (m_max_radius * 2);
            particle.m_scale = glm::lerp(0.25f, 1.f, p);
            curl_fade = p;
        }
        // curl-noise
        float lx = (particle.m_pos.x / m_max_radius + 1) / 2.f;
        float ly = (particle.m_pos.y / m_max_radius + 1) / 2.f;
        lx = glm::clamp(lx, 0.f, 1.f);
        ly = glm::clamp(ly, 0.f, 1.f);
        auto curl_noise = ResourceManager::Instance().get_smoke_curl_noise();
        size_t nx = lx * (curl_noise.size() - 1) + 0.5;
        size_t ny = ly * (curl_noise.size() - 1) + 0.5;
        particle.m_pos += curl_fade * curl_noise[nx][ny] * 20.f * (float)(dtime / (1000.f * 1000.f));
        if (particle.m_timer >= m_particle_produce_duration + m_particle_flow_duration) {
            float p = glm::clamp((particle.m_timer - m_particle_produce_duration - m_particle_flow_duration) / m_particle_fade_duration, 0.f, 1.f);
            particle.m_fade = smoothstep(1 - p);
        }
        particle.m_rot += 20.f * particle.m_tor_vel * (dtime / (1000.f * 1000.f));
    }

    if (m_global_timer <= m_particle_produce_duration) {
        int N = (m_global_timer / m_particle_produce_duration) * m_max_particles;
        for (size_t i = m_particles.size(); i < N; ++i)
            m_particles.push_back(glm::diskRand(1.f));
    }
    return true;
}

bool Smoke::active() const {
    return m_global_timer >= m_activation_time && m_global_timer <= m_deactivation_time;
}

void Smoke::draw(sf::RenderWindow& window) {
    if (!m_enabled) return;
    for (auto it = m_particles.rbegin(); it != m_particles.rend(); ++it) {
        break;
        auto& particle = *it;
        m_particle_sprite.setPosition(m_pos.x + particle.m_pos.x, m_pos.y + particle.m_pos.y);
        m_particle_sprite.setScale(particle.m_scale, particle.m_scale);
        m_particle_sprite.setTextureRect(sf::IntRect((particle.texture_id % 4) * 128, (particle.texture_id / 4) * 128, 128, 128));
        m_particle_sprite.setOrigin(64, 64);
        m_particle_sprite.setColor(sf::Color(255, 255, 255, particle.m_fade * 255.f));
        m_particle_sprite.setRotation(particle.m_rot);
        window.draw(m_particle_sprite);
    }

    if (DEBUG_ENABLED && active()) {
        sf::CircleShape circ(m_max_radius, 40);
        circ.setOrigin(m_max_radius, m_max_radius);
        circ.setPosition(m_pos.x, m_pos.y);
        circ.setOutlineThickness(1);
        circ.setOutlineColor(sf::Color::Red);
        circ.setFillColor(sf::Color::Transparent);
        window.draw(circ);
    }

}
