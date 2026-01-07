#include "enemies/cruiser_I.h"
#include "shader_manager.h"
#include "sound_manager.h"
#include "utils/framers.h"
#include "enemies/simple_enemy.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>

CruiserTurretAnimation::CruiserTurretAnimation(const CruiserI& cruiser) {
    m_turret_sprite.setTexture(TextureManager::Instance().textures[TextureID::CruiserISupplementary]);
    m_turret_sprite.setTextureRect(sf::IntRect(0,0,16,16));
    m_turret_sprite.setOrigin(5, 8);
    m_turret_sprite.setScale(0.5, 0.5);
    m_sep_position = glm::vec2{ cruiser.position.x,  cruiser.position.y } + glm::rotate(glm::vec2{-6 / 2., -11/2.}, glm::radians(cruiser.rotation));
    m_turret_sprite.setPosition(m_sep_position.x, m_sep_position.y);
    m_rotation = cruiser.rotation;
    m_turret_sprite.setRotation(cruiser.rotation);
    m_vel = glm::sphericalRand(20.); // определим начальный вектор скорости
    m_vel.z = glm::abs(m_vel.z);
}

void CruiserTurretAnimation::draw(sf::RenderWindow& window) {
    m_turret_sprite.setPosition(m_position.x + m_sep_position.x, m_position.y + m_sep_position.y);
    m_turret_sprite.setColor(sf::Color(255, 255, 255, m_fade * 255));
    m_turret_sprite.setRotation(m_rotation);
    float scale_factor = (m_position.z / (m_vel.z * m_vel.z) * (2 * 9.8));
    scale_factor = 0.5 * (1. + scale_factor);
    m_turret_sprite.setScale(scale_factor, scale_factor);
    window.draw(m_turret_sprite);
}

bool CruiserTurretAnimation::logic(double dtime_mc) {
    if (m_state == State::Fly) {
        m_timer += dtime_mc;
        float dt = dtime_mc / (1000.f * 1000.f);
        float t = m_timer / (1000.f * 1000.f);
        m_rotation += dt * 100;
        m_position = -glm::vec3(0, 0, 9.8f) * t * t / 2.f + m_vel * t;
        if (m_position.z < 0) {
            m_position.z = 0;
            m_state = State::Land;
            m_timer = 0;
        }
        return false;
    }
    else if (m_state == State::Land) {
        m_timer += dtime_mc;
        if (m_timer >= 2 * 1000 * 1000) {
            m_state = State::Fade;
            m_timer = 0;
        }
        return false;
    }
    else if (m_state == State::Fade) {
        m_timer += dtime_mc;
        m_fade = 1. - m_timer / (2 * 1000 * 1000.f);
        return m_timer >= 2 * 1000 * 1000;
    }
}


CruiserI::CruiserI() : IEnemy(ParamsManager::Instance().params.enemies.CruiserI, EnemyType::CruiserI, Collision(glm::vec2(-13.5, -8.25), glm::vec2(15, 8.25))) {
    wheels = Wheels::HeavyTracks;
    infantry = false;
    m_cruiserI.sprite.setTexture(TextureManager::Instance().textures[TextureID::CruiserIBase]);
    m_cruiserI.sprite.setScale(0.5, 0.5);
    m_cruiserI.layer = 0;
    m_cruiserI.set_position_origin(16, 16.25);
    m_cruiserI.set_rotation_origin(16, 16.25);
    m_upper_truck = &m_cruiserI.childs.emplace_back();
    m_lower_truck = &m_cruiserI.childs.emplace_back();


    auto& shader = ShaderManager::Instance().shaders[Shader::Scroll];

    m_upper_truck->sprite.setTexture(TextureManager::Instance().textures[TextureID::Trucks]);
    m_upper_truck->sprite.setTextureRect(sf::IntRect(0, 0, 56, 7));
    m_upper_truck->sprite.setScale(5 / 14.f, 5 / 14.f);
    m_upper_truck->set_position(10 / 2., 16 / 2.);
    m_upper_truck->shader = &shader;
    m_upper_truck->layer = 1;

    m_lower_truck->sprite.setTexture(TextureManager::Instance().textures[TextureID::Trucks]);
    m_lower_truck->sprite.setTextureRect(sf::IntRect(0, 0, 56, 7));
    m_lower_truck->sprite.setScale(5 / 14.f, 5 / 14.f);
    m_lower_truck->set_position(10 / 2., 44 / 2.);
    m_lower_truck->shader = &shader;
    m_lower_truck->layer = 1;

    m_equipment = &m_cruiserI.childs.emplace_back();
    m_equipment->sprite.setTexture(TextureManager::Instance().textures[TextureID::CruiserIEquipment]);
    m_equipment->sprite.setScale(0.5, 0.5);
    m_equipment->layer = 2;


    m_fire_sprite = &m_cruiserI.childs.emplace_back();
    m_fire_sprite->layer = 3;
    m_fire_sprite->set_position(16 / 2.f, 28 / 2.f);

    m_dence_blust_sprite = &m_cruiserI.childs.emplace_back();
    m_dence_blust_sprite->set_position(16 / 2.f, 28 / 2.f);
    m_dence_blust_sprite->layer = 4;

    m_fire_framer = std::make_unique<CruiserIFireFramer>();
    m_fire_animation.add_framer(m_fire_framer);
    m_fire_animation.set_loop(true);
    m_fire_animation.set_duration(1.);

    m_dence_blust_framer = std::make_unique<DenceBlustFramer>();
    m_dence_blust_animation.add_framer(m_dence_blust_framer);
    m_dence_blust_animation.set_duration(1.5);

}

void CruiserI::make_boss() {
    m_indicator.width = 32;
    m_indicator.fill_color = sf::Color::Magenta;
    IEnemy::make_boss();
}

void CruiserI::draw(sf::RenderWindow& window) {
    auto& shader = ShaderManager::Instance().shaders[Shader::Scroll];
    shader.setUniform("texture", sf::Shader::CurrentTexture);
    shader.setUniform("offset_x", -(float)m_trucks_offset / (1000 * 1000));
    shader.setUniform("offset_y", 0.f);
    m_cruiserI.set_rotation(rotation);
    m_cruiserI.set_position(position.x, position.y);
    if (m_fire_animation.started()) {
        m_fire_sprite->sprite = m_fire_framer->sprite;
    }
    if (m_dence_blust_animation.started()) {
        m_dence_blust_sprite->sprite = m_dence_blust_framer->sprite;
        m_dence_blust_sprite->enabled = true;
    }
    else
        m_dence_blust_sprite->enabled = false;
    m_cruiserI.draw(window);
}

void CruiserI::draw_effects(sf::RenderWindow& window) {
    IEnemy::draw_effects(window);
    m_indicator.draw(window, position.x, position.y - (rotation ? 16 : 10), params.health, health);
}

bool CruiserI::logic(double dtime_microseconds) {
    m_fire_animation.logic(dtime_microseconds);
    m_dence_blust_animation.logic(dtime_microseconds);

    m_trucks_offset += params.speed * dtime_microseconds;
    if (m_trucks_offset >= 1000 * 1000)
        m_trucks_offset = 0;
    if (health <= params.health / 2 && m_first_stage) {
        m_first_stage = false;
        m_cruiserI.sprite.setTexture(TextureManager::Instance().textures[TextureID::CruiserIDemagedBase]);
        m_equipment->sprite.setTexture(TextureManager::Instance().textures[TextureID::CruiserIDemagedEquipment]);
        m_fire_animation.start();
        m_dence_blust_animation.start();
        m_fire_animation.logic(0);
        m_dence_blust_animation.logic(0);
        SoundManager::Instance().play(Sounds::DenceBlust);
        AnimationHolder::Instance().add_object(std::make_unique<CruiserTurretAnimation>(*this));
    }
    return IEnemy::logic(dtime_microseconds);
}

IDestroyedEnemy::Ptr CruiserI::get_destroyed_enemy() {
    auto de = std::make_unique<SimpleEnemyDestroyed>(std::make_unique<CruiserIBlustFramer>(), 2, 2.0, Sounds::CruiserIExplosion);
    de->destroyed_enemy_sprite.setScale(0.5, 0.5);
    de->destroyed_enemy_sprite.setOrigin(32, 32);
    de->destroyed_enemy_sprite.setPosition(position.x, position.y);
    de->destroyed_enemy_sprite.setRotation(rotation);
    de->destroyed_enemy_sprite.setTexture(TextureManager::Instance().textures[TextureID::CruiserIDestroyed]);
    return de;
}
