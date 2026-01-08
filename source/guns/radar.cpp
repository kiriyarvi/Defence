#include "guns/radar.h"
#include <enemy_manager.h>
#include <covering_database.h>
#include <debugger.h>

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

    if (DEBUG_ENABLED) {
        for (auto& t : m_targets) {
            IEnemy* enemy = EnemyManager::Instance().get_enemy_by_id(t.target);
            Debugger::Instance().add_line({ x_id * 32 + 16, y_id * 32 + 16 }, enemy->position, sf::Color::Blue);
            Debugger::Instance().add_indicator(t.target, t.uncovering_time / (m_params.uncover_time * 1000 * 1000));
        }
    }
}

void Radar::draw_effects(sf::RenderWindow& window, int x_id, int y_id) {
    if (DEBUG_ENABLED) {
        sf::CircleShape circ(m_params.radius * 32);
        circ.setOrigin(m_params.radius * 32, m_params.radius * 32);
        circ.setPosition(x_id * 32 + 16, y_id * 32 + 16);
        circ.setFillColor(sf::Color::Transparent);
        circ.setOutlineThickness(1);
        circ.setOutlineColor(sf::Color::Green);
        window.draw(circ);
    }
}


void Radar::logic(double dtime_microseconds, int x_id, int y_id) {
    m_rotation += dtime_microseconds / (1000.f * 1000.f) * 20.f;
    glm::vec2 pos = { x_id * 32 + 16, y_id * 32 + 16 };

    if (m_status == Status::Aiming) {
        m_aiming_timer += dtime_microseconds;
        if (m_aiming_timer >= m_params.aiming_time * 1000.f * 1000.f)
            m_status = Status::Uncover;
    }

    // удаляем несуществующие цели + увеличиваем таймер. Удаляем, если цель раскрыта
    auto& enemy_manager = EnemyManager::Instance();
    for (auto it = m_targets.begin(); it != m_targets.end();) {
        IEnemy* enemy;
        if (!(enemy = enemy_manager.get_enemy_by_id(it->target))) { // враг больше не существует.
            it = m_targets.erase(it);
            continue;
        }
        if (!enemy->m_in_smoke) { // враг вышел из дымовой завесы.
            it = m_targets.erase(it);
            continue;
        }
        it->uncovering_time += dtime_microseconds;
        if (it->uncovering_time >= m_params.uncover_time * 1000 * 1000) {
            CoveringDataBase::Instance().make_enemy_uncovered(it->target);
            it = m_targets.erase(it);
        } else
            ++it;
    }
 

    if (m_status != Status::Aiming && m_targets.size() < m_params.max_targets) {
        // поиск новых целей.
        IEnemy* enemy = nullptr;
        uint32_t id;
        float metric;
        for (auto& e : enemy_manager.m_enemies) {
            if (CoveringDataBase::Instance().is_available_taget(e->id)) continue;
            float d = glm::distance(e->position, pos);
            if (d >= m_params.radius * 32) continue;
            if (std::find_if(m_targets.begin(), m_targets.end(), [&](Target& t) {return t.target == e->id; }) != m_targets.end())
                continue;
            float m = d + (e->position.x - pos.x); // чем левее, тем выше приоритет.
            if (enemy == nullptr || m >= metric) {
                enemy = e.get();
                id = enemy->id;
                metric = m;
            }
        }
        if (enemy != nullptr) {
            m_status = Status::Aiming;
            m_aiming_timer = 0;
            m_targets.push_back({ id, 0.0 });
        }
    }

 
}
