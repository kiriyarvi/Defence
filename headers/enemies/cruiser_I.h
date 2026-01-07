#pragma once
#include "IEnemy.h"
#include "utils/sprite_chain.h"
#include "utils/animation.h"
#include "animation_holder.h"

class CruiserI;

class CruiserTurretAnimation : public IAnimationObject {
public:
    CruiserTurretAnimation(const CruiserI& cruiser);
    void draw(sf::RenderWindow& window) override;
    bool logic(double dtime_mc) override; // true --- значит конец.
private:
    enum class State {
        Fly,
        Land,
        Fade
    } m_state = State::Fly;

    sf::Sprite m_turret_sprite;
    glm::vec3 m_position;
    glm::vec2 m_sep_position;
    glm::vec3 m_vel;
    float m_rotation;
    float m_timer = 0;
    float m_fade = 1.;
};


class CruiserI : public IEnemy {
public:
    CruiserI();
    void draw(sf::RenderWindow& window) override;
    void draw_effects(sf::RenderWindow& window) override;
    bool logic(double dtime_microseconds) override;
    IDestroyedEnemy::Ptr get_destroyed_enemy();
    void make_boss() override;
private:
    double m_trucks_offset = 0;
    bool m_first_stage = true;
    SpriteChain m_cruiserI;
    SpriteChain* m_upper_truck;
    SpriteChain* m_lower_truck;
    SpriteChain* m_equipment;
    HealthIndicator m_indicator;
    Animation m_fire_animation;
    ISpriteFramer::Ptr m_fire_framer;
    SpriteChain* m_fire_sprite;

    Animation m_dence_blust_animation;
    ISpriteFramer::Ptr m_dence_blust_framer;
    SpriteChain* m_dence_blust_sprite;
};
