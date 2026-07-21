#include "guns/radar.h"
#include <enemy_manager.h>
#include <covering_database.h>
#include <debugger.h>

Radar::Radar(int x_id, int y_id):IBuilding(x_id, y_id), m_params(ParamsManager::Instance().params.guns.radar) {
    m_radar_sprite.setTexture(TextureManager::Instance().textures[TextureID::Radar]);
    m_radar_sprite.setOrigin(16, 16);

    m_base_sprite.setTexture(TextureManager::Instance().textures[TextureID::GunBase]);
    m_base_sprite.setOrigin(16, 16);
}

void Radar::draw(sf::RenderWindow& window) {
    m_base_sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    window.draw(m_base_sprite);
    m_radar_sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    m_radar_sprite.setRotation(m_rotation);
    window.draw(m_radar_sprite);

    if (DEBUG_ENABLED) {
        float uncover_time = m_params.uncovering_speed_upgrades[uncovering_speed_upgrade].uncover_time;
        for (auto& t : m_targets) {
            IEnemy* enemy = EnemyManager::Instance().get_enemy_by_id(t.target);
            Debugger::Instance().add_line({ x_id * 32 + 16, y_id * 32 + 16 }, enemy->position, sf::Color::Blue);
            Debugger::Instance().add_indicator(t.target, t.uncovering_time / (uncover_time * 1000 * 1000));
            auto status = CoveringDataBase::Instance().get_status(t.target);
            Debugger::Instance().add_text(
                std::to_string(status.covering_level) + "/" + std::to_string(status.uncovering_level),
                enemy->position + glm::vec2{8.0, 0.5});
        }
    }
}

void Radar::draw_effects(sf::RenderWindow& window) {
    if (DEBUG_ENABLED && false) {
        int radius = m_params.radius_upgrades[radius_upgrade].radius;
        sf::CircleShape circ(radius * 32);
        circ.setOrigin(radius * 32, radius * 32);
        circ.setPosition(x_id * 32 + 16, y_id * 32 + 16);
        circ.setFillColor(sf::Color::Transparent);
        circ.setOutlineThickness(1);
        circ.setOutlineColor(sf::Color::Green);
        window.draw(circ);
    }
}

void Radar::upgrade_radius(int level) {
    radius_upgrade = level;
}

void Radar::upgrade_uncovering_level(int level) {
    uncovering_level_upgrade = level;
}

void Radar::upgrade_uncovering_speed(int level) {
    uncovering_speed_upgrade = level;
}

void Radar::upgrade_long_distance_communication() {
    long_distance_communication_upgrade = 1;
}

void Radar::logic(double dtime_microseconds) {
    m_rotation += dtime_microseconds / (1000.f * 1000.f) * 20.f;
    if (m_part_of_net)
        return; //Остальную логику берет на себя сеть

    glm::vec2 pos = { x_id * 32 + 16, y_id * 32 + 16 };

    float aiming_time = m_params.uncovering_speed_upgrades[uncovering_speed_upgrade].aiming_time;
    float uncover_time = m_params.uncovering_speed_upgrades[uncovering_speed_upgrade].uncover_time;
    float radius = m_params.radius_upgrades[radius_upgrade].radius;
    float uncovering_level = m_params.uncovering_level_upgrades[uncovering_level_upgrade].uncovering_level;

    if (m_status == Status::Aiming) {
        m_aiming_timer += dtime_microseconds;
        if (m_aiming_timer >= aiming_time * 1000.f * 1000.f)
            m_status = Status::Uncover;
    }

    // удаляем несуществующие цели + увеличиваем таймер. Удаляем, если цель раскрыта
    auto& enemy_manager = EnemyManager::Instance();
    for (auto it = m_targets.begin(); it != m_targets.end();) {
        IEnemy* enemy; 
        if (!enemy_manager.get_enemy_by_id(it->target)) { // враг больше не существует. TODO это место очень узкое
            it = m_targets.erase(it);
            continue;
        }
        auto enemy_status = CoveringDataBase::Instance().get_status(it->target);
        if (enemy_status.uncovering_level >= enemy_status.covering_level) { // враг раскрыт (возможно другим радаром).
            it = m_targets.erase(it);
            continue;
        }
        it->uncovering_time += dtime_microseconds;
        if (it->uncovering_time >= uncover_time * 1000 * 1000) {
            CoveringDataBase::Instance().enemy_uncovering_level(it->target, uncovering_level);
            it = m_targets.erase(it);
        } else
            ++it;
    }
 

    if (m_status != Status::Aiming && m_targets.size() < m_params.max_targets) {
        // поиск новых целей.
        IEnemy* enemy = nullptr;
        float metric = 1.;
        for (auto& e : enemy_manager.m_enemies) {
            if (e->m_covering_level > uncovering_level)
                continue; //не хватает uncovering для раскрытия цели
            if (CoveringDataBase::Instance().is_available_taget(e->id))
                continue; //цель рассекречена => пропускаем
            if (std::find_if(m_targets.begin(), m_targets.end(), [&](Target& t) {return t.target == e->id; }) != m_targets.end())
                continue; // цель уже захвачена
            float d = glm::distance(e->position, pos);
            if (d >= radius * 32)
                continue; // цель слишком далеко
            
            if ((enemy == nullptr || e->path_progress < metric) && e->m_covering_level <= uncovering_level) {
                enemy = e.get();
                metric = e->path_progress;
            }
        }
        if (enemy != nullptr) {
            m_status = Status::Aiming;
            m_aiming_timer = 0;
            m_targets.push_back({ enemy->id, 0.0 });
        }
    }

 
}

Radar::~Radar() {
    NetManager::Instance().radar_deleted(x_id, y_id);
}
