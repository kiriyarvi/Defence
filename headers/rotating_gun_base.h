#pragma once
#include "tile_map.h"

class Enemy;

class IRotatingGun : public IBuilding {
public:
	void draw(sf::RenderWindow& window, int x_id, int y_id) override;
	void logic(double dtime_microseconds, int x_id, int y_id) override;
	virtual void shoot_logic(int x_id, int y_id, Enemy& enemy) = 0;

	double rotation = 0; //текущее вращение.
	double rotation_speed = 90; // скорость вращения (градусы в секунду).
	double radius; // радиус действия (указывается в клетках)
	std::unique_ptr<IRotatingGun> gun = nullptr;
	TileTexture base_texture = TileTexture::GunBase; // текстура основания орудия.
private:
	bool is_enemy_captured = false;
	uint32_t captured_enemy_id;
};

