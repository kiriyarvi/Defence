#pragma once
#include "guns/building.h"
#include "texture_manager.h"

class IEnemy;

// TODO добавить возможность указания приоритета целей.
class IRotatingGun : public IBuilding {
public:
    IRotatingGun(int x_id, int y_id, BuildingType type);
	void draw(sf::RenderWindow& window) override;
	void logic(double dtime_microseconds) override;
	virtual void shoot_logic(IEnemy& enemy) = 0;
	double rotation = 180; //текущее вращение.
	double rotation_speed = 90; // скорость вращения (градусы в секунду).
	double radius; // радиус действия (указывается в клетках)
	TextureID base_texture = TextureID::GunBase; // текстура основания орудия.
protected:
	virtual void on_gun_pointed() {}
	virtual void on_gun_unpointed() {}
    struct TargetStatus {
        bool valid = true;
        float priority = 1.;
        bool mult_by_distance = true;
    };
    virtual TargetStatus get_enemy_status(IEnemy& enemy) = 0; // определяет приоритет цели по этой функции (также может отбраковать). 
protected:
	bool m_is_gun_pointed = false;
	bool m_is_enemy_captured = false;
	uint32_t m_captured_enemy_id;
};

