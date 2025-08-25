#pragma once

#include <nlohmann/json.hpp>

class ParamsManager {
public:
	static ParamsManager& Instance() {
		static ParamsManager instance; // �������� ��� ������ ������, ��������������� � C++11+
		return instance;
	}

	// ������� ����������� � �����������
	ParamsManager(const ParamsManager&) = delete;
	ParamsManager& operator=(const ParamsManager&) = delete;
	ParamsManager(ParamsManager&&) = delete;
	ParamsManager& operator=(ParamsManager&&) = delete;

	struct Params {
		struct Guns {
			struct Antitank {
				float radius;
				float damage;
				float cooldown;
				float cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Antitank, radius, damage, cooldown, cost)
			} antitank;
			struct TwinGun {
				float radius;
				float damage_per_barrel;
				float cooldown;
				float interleaved_cooldown;
				float cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(TwinGun, radius, damage_per_barrel, cooldown, interleaved_cooldown, cost)
			} twingun;
			struct Minigun {
				float radius;
				float heating_time;
				float cooling_time;
				float min_rotation_speed;
				float max_rotation_speed;
				float min_damage;
				float max_damage;
				float cooldown_duration;
				float critical_temperature;
				float critical_temperature_work_duration;
				float cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Minigun,
					radius, heating_time, cooling_time,
					min_rotation_speed, max_rotation_speed,
					min_damage, max_damage,
					cooldown_duration, critical_temperature,
					critical_temperature_work_duration,
					cost
				)
			} minigun;
			struct Mine {
				float damage_radius;
				float activation_radius;
				float min_damage;
				float max_damage;
				float cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mine, damage_radius, activation_radius, min_damage, max_damage, cost)
			} mine;
			struct Spikes {
				int health;
				float delay;
				float cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Spikes, health, delay, cost)
			} spikes;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(Guns, antitank, twingun, minigun, mine, spikes)
		} guns;
		struct Enemies {
			struct Enemy {
				float speed;
				float health;
				float reward;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Enemy, speed, health, reward)
			};
			Enemy solder;
			Enemy bike;
			Enemy truck;
			Enemy tank;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(Enemies, solder, bike, truck, tank)
		} enemies;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, guns, enemies)
	} params;
private:
	ParamsManager();
};