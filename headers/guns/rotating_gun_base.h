#pragma once
#include "tile_map.h"

class IEnemy;

class IRotatingGun : public IBuilding {
public:
	void draw(sf::RenderWindow& window, int x_id, int y_id) override;
	void logic(double dtime_microseconds, int x_id, int y_id) override;
	virtual void shoot_logic(int x_id, int y_id, IEnemy& enemy) = 0;

	double rotation = 180; //������� ��������.
	double rotation_speed = 90; // �������� �������� (������� � �������).
	double radius; // ������ �������� (����������� � �������)
	TileTexture base_texture = TileTexture::GunBase; // �������� ��������� ������.
private:
	bool is_enemy_captured = false;
	uint32_t captured_enemy_id;
};

