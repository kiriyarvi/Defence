#pragma once
#include "tile_map.h"

class IEnemy;

// TODO добавить возможность указания приоритета целей.
class IRotatingGun : public IBuilding {
public:
	void draw(sf::RenderWindow& window, int x_id, int y_id) override;
	void logic(double dtime_microseconds, int x_id, int y_id) override;
	virtual void shoot_logic(int x_id, int y_id, IEnemy& enemy) = 0;
	double rotation = 180; //текущее вращение.
	double rotation_speed = 90; // скорость вращения (градусы в секунду).
	double radius; // радиус действия (указывается в клетках)
	TileTexture base_texture = TileTexture::GunBase; // текстура основания орудия.
protected:
	virtual void on_gun_pointed() {}
	virtual void on_gun_unpointed() {}
	bool m_is_gun_pointed = false;
	bool m_is_enemy_captured = false;
	uint32_t m_captured_enemy_id;
};

