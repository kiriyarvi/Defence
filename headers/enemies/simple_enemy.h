#pragma once
#include "enemy_manager.h"

class SimpleEnemy : public IEnemy {
public:
	SimpleEnemy(EnemyTexturesID enemy_texture, EnemyTexturesID destroyed_enemy_texture);
	DestroyedEnemy get_destroyed_enemy() override;
	void draw(sf::RenderWindow& window) override;
protected:
	sf::Sprite m_enemy_sprite;
	EnemyTexturesID m_destroyed_enemy_texture;
	HealthIndicator m_indicator;
};

class Tank : public SimpleEnemy {
public:
	Tank() : SimpleEnemy(EnemyTexturesID::Tank, EnemyTexturesID::TankDestroyed) {
		health = full_health = 100;
		speed = 0.4;
	}
};

class Truck : public SimpleEnemy {
public:
	Truck() : SimpleEnemy(EnemyTexturesID::Truck, EnemyTexturesID::TruckDestroyed) {
		health = full_health = 50;
		speed = 0.8;
	}
};

class Bike : public SimpleEnemy {
public:
	Bike() : SimpleEnemy(EnemyTexturesID::Bike, EnemyTexturesID::DestroyedBike) {
		health = full_health = 30;
		speed = 1.6;
		m_enemy_sprite.setOrigin(64, 64);
		m_enemy_sprite.setScale(0.25, 0.25);
	}

	DestroyedEnemy get_destroyed_enemy() {
		auto de = SimpleEnemy::get_destroyed_enemy();
		de.sprite.setScale(0.3, 0.3);
		return de;
	}
};