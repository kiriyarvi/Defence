#pragma once
#include "enemy_manager.h"
#include "utils/animation.h"
#include "utils/framers.h"
#include "sound_manager.h"

class SimpleEnemy : public IEnemy {
public:
	SimpleEnemy(EnemyTexturesID enemy_texture, EnemyTexturesID destroyed_enemy_texture, Sounds destruction_sound, const ParamsManager::Params::Enemies::Enemy& params);
	IDestroyedEnemy::Ptr get_destroyed_enemy() override;
	void draw(sf::RenderWindow& window) override;
protected:
	sf::Sprite m_enemy_sprite;
	EnemyTexturesID m_destroyed_enemy_texture;
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
	Tank() : SimpleEnemy(EnemyTexturesID::Tank, EnemyTexturesID::TankDestroyed, Sounds::DoubleBlust, ParamsManager::Instance().params.enemies.tank) {
		wheels = Wheels::Tracks;
		infantry = false;
	}

	IDestroyedEnemy::Ptr get_destroyed_enemy() {
		auto de = std::make_unique<SimpleEnemyDestroyed>(std::make_unique<DoubleBlustFramer>(), 1.4, 2.0, Sounds::DoubleBlust);
		de->destroyed_enemy_sprite = m_enemy_sprite;
		de->destroyed_enemy_sprite.setTexture(EnemyManager::Instance().enemy_textures[m_destroyed_enemy_texture]);
		return de;
	}
};

class Truck : public SimpleEnemy {
public:
	Truck() : SimpleEnemy(EnemyTexturesID::Truck, EnemyTexturesID::TruckDestroyed, Sounds::MedBlustOfDestruction, ParamsManager::Instance().params.enemies.truck) {
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
	Bike() : SimpleEnemy(EnemyTexturesID::Bike, EnemyTexturesID::DestroyedBike, Sounds::MedBlustOfDestruction, ParamsManager::Instance().params.enemies.bike) {
		m_enemy_sprite.setOrigin(64, 64);
		m_enemy_sprite.setScale(0.25, 0.25);
		wheels = Wheels::Wheels;
		infantry = true;
	}
	IDestroyedEnemy::Ptr get_destroyed_enemy() override;
};