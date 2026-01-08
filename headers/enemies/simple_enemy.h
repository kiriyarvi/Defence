#pragma once
#include "enemy_manager.h"
#include "utils/animation.h"
#include "utils/framers.h"
#include "sound_manager.h"
#include "utils/sprite_chain.h"

class SimpleEnemy : public IEnemy {
public:
	SimpleEnemy(TextureID enemy_texture, TextureID destroyed_enemy_texture, Sounds destruction_sound, const ParamsManager::Params::Enemies::Enemy& params, EnemyType t, Collision c);
	IDestroyedEnemy::Ptr get_destroyed_enemy() override;
	void draw(sf::RenderWindow& window) override;
    void draw_effects(sf::RenderWindow& window) override;
protected:
	sf::Sprite m_enemy_sprite;
    TextureID m_destroyed_enemy_texture;
	HealthIndicator m_indicator;
	Sounds m_destruction_sound;
};

class SimpleEnemyDestroyed : public IDestroyedEnemy {
public:
	SimpleEnemyDestroyed(ISpriteFramer::Ptr&& framer, double blust_time, double fade_time, Sounds destruction_sound);
	void draw(sf::RenderWindow& window) override;
	void logic(double dtime_microseconds) override;
	bool is_ready() override;
	sf::Sprite destroyed_enemy_sprite;
private:
	ISpriteFramer::Ptr m_framer;
	Animation m_animation;
	bool m_draw_blust = true;
};


class Tank : public SimpleEnemy {
public:
	Tank() : SimpleEnemy(TextureID::Tank, TextureID::TankDestroyed, Sounds::DoubleBlust, ParamsManager::Instance().params.enemies.tank, EnemyType::Tank, Collision(glm::vec2(-10,-6), glm::vec2(15, 6))) {
		wheels = Wheels::Tracks;
		infantry = false;
	}

	IDestroyedEnemy::Ptr get_destroyed_enemy() {
		auto de = std::make_unique<SimpleEnemyDestroyed>(std::make_unique<DoubleBlustFramer>(), 1.4, 2.0, Sounds::DoubleBlust);
		de->destroyed_enemy_sprite = m_enemy_sprite;
		de->destroyed_enemy_sprite.setTexture(TextureManager::Instance().textures[m_destroyed_enemy_texture]);
		return de;
	}
};

class Truck : public SimpleEnemy {
public:
	Truck() : SimpleEnemy(TextureID::Truck, TextureID::TruckDestroyed, Sounds::MedBlustOfDestruction, ParamsManager::Instance().params.enemies.truck, EnemyType::Truck, Collision(glm::vec2(-11,-4), glm::vec2(12, 4))) {
		wheels = Wheels::Wheels;
		infantry = false;
	}
};

class BikeDestroyed : public IDestroyedEnemy {
public:
	BikeDestroyed();
	void draw(sf::RenderWindow& window) override;
	void logic(double dtime_microseconds) override;
	bool is_ready() override;
	sf::Sprite destroyed_enemy_sprite;
private:
	struct BlustInfo {
		bool active = false;
		sf::Vector2f offset;
		ISpriteFramer::Ptr framer;
	};
	std::array< BlustInfo, 3> m_blusts_info;

	Animation m_animation;
	bool m_draw_blust = true;
};

class Bike : public SimpleEnemy {
public:
	Bike() : SimpleEnemy(TextureID::Bike, TextureID::DestroyedBike, Sounds::MedBlustOfDestruction, ParamsManager::Instance().params.enemies.bike, EnemyType::Bike, Collision(glm::vec2(-7.75, -3.25), glm::vec2(11, 3.25))) {
		m_enemy_sprite.setOrigin(64, 64);
		m_enemy_sprite.setScale(0.25, 0.25);
		wheels = Wheels::Wheels;
		infantry = true;
	}
	IDestroyedEnemy::Ptr get_destroyed_enemy() override;
};

class Pickup : public SimpleEnemy {
public:
    Pickup() : SimpleEnemy(TextureID::Pickup, TextureID::PickupDestroyed, Sounds::PickupDestruction, ParamsManager::Instance().params.enemies.pickup, EnemyType::Pickup, Collision(glm::vec2(-9, -4), glm::vec2(11, 4))) {
        wheels = Wheels::Wheels;
        infantry = false;
    }
    IDestroyedEnemy::Ptr get_destroyed_enemy() {
        auto de = std::make_unique<SimpleEnemyDestroyed>(std::make_unique<PickupBlastFramer>(), 1.4, 2.0, Sounds::PickupDestruction);
        de->destroyed_enemy_sprite = m_enemy_sprite;
        de->destroyed_enemy_sprite.setTexture(TextureManager::Instance().textures[m_destroyed_enemy_texture]);
        return de;
    }
};

class BTR : public IEnemy {
public:
    BTR();
    void draw(sf::RenderWindow& window) override;
    void draw_effects(sf::RenderWindow& window) override;
    bool logic(double dtime_microseconds) override;
    IDestroyedEnemy::Ptr get_destroyed_enemy();
private:
    double m_trucks_offset = 0;
    SpriteChain m_btr;
    SpriteChain* m_upper_truck;
    SpriteChain* m_lower_truck;
    HealthIndicator m_indicator;
};


class SmokeTruck : public SimpleEnemy {
public:
    SmokeTruck() : SimpleEnemy(TextureID::SmokeTruck, TextureID::SmokeTruckDestroyed, Sounds::MedBlustOfDestruction, ParamsManager::Instance().params.enemies.smoke_truck, EnemyType::SmokeTruck, Collision(glm::vec2(-13,-5), glm::vec2(12, 5))) {
        wheels = Wheels::Wheels;
        infantry = false;
    }
    IDestroyedEnemy::Ptr get_destroyed_enemy() override {
        EnemyManager::Instance().add_smoke(Smoke(get_position(), 4., 20.));
        return SimpleEnemy::get_destroyed_enemy();
    }
};

class MREW : public SimpleEnemy {
public:
    // TODO уничтоженная версия.
    MREW() : SimpleEnemy(TextureID::MREW, TextureID::MREW, Sounds::MedBlustOfDestruction, ParamsManager::Instance().params.enemies.mrew.enemy_params, EnemyType::MREW, Collision(glm::vec2(-11, -6), glm::vec2(11, 6))) {
        m_radar.setTexture(TextureManager::Instance().textures[TextureID::MREWRadar]);
        m_radar.setTextureRect(sf::IntRect(5, 10, 9, 9));
        m_radar.setOrigin(4.5, 4.5);
    }
    void draw(sf::RenderWindow& window) override;
    bool logic(double dtime) override;
private:
    float m_radar_rotation = 0;
    sf::Sprite m_radar;
};

