#pragma once

#include <nlohmann/json.hpp>

class ParamsManager {
public:
	static ParamsManager& Instance() {
		static ParamsManager instance; // Создаётся при первом вызове, потокобезопасно в C++11+
		return instance;
	}

	// Удаляем копирование и перемещение
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
                int cost;
                int armor_penetration_level;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Antitank, radius, damage, cooldown, cost, armor_penetration_level)
			} antitank;
			struct TwinGun {
				float radius;
				float damage_per_barrel;
				float cooldown;
				float interleaved_cooldown;
                int cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(TwinGun, radius, damage_per_barrel, cooldown, interleaved_cooldown, cost)
			} twingun;
			struct Minigun {
				float radius;
				float heating_time;
				float cooling_time;
				float min_rotation_speed;
				float max_rotation_speed;
				int min_damage;
				int max_damage;
				float cooldown_duration;
				float critical_temperature;
				float critical_temperature_work_duration;
				int cost;
				int min_armor_penetration_level;
				int max_armor_penetration_level;
				struct PenetrationUpgrade {
					int min_armor_penetration_level;
					int max_armor_penetration_level;
					NLOHMANN_DEFINE_TYPE_INTRUSIVE(PenetrationUpgrade, min_armor_penetration_level, max_armor_penetration_level)
				};
				std::vector<PenetrationUpgrade> penetration_upgrades;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Minigun,
					radius, heating_time, cooling_time,
					min_rotation_speed, max_rotation_speed,
					min_damage, max_damage,
					cooldown_duration, critical_temperature,
					critical_temperature_work_duration,
					cost, penetration_upgrades
				)
			} minigun;
			struct Mine {
				float damage_radius;
				float activation_radius;
				int min_damage;
				int max_damage;
                int armor_penetration_level;
                int cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mine, damage_radius, activation_radius, min_damage, max_damage, cost, armor_penetration_level)
			} mine;
			struct Spikes {
				int health;
				float delay;
                int cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Spikes, health, delay, cost)
			} spikes;
			struct Hedgehog {
				int health;
				float delay;
                int cost;
				float wheels_debuff;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Hedgehog, health, delay, cost, wheels_debuff)
			} hedgehog;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(Guns, antitank, twingun, minigun, mine, spikes, hedgehog)
		} guns;
		struct Enemies {
			struct Enemy {
				float speed;
				float health;
				float reward;
				int armor_level;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Enemy, speed, health, reward, armor_level)
			};
			Enemy solder;
			Enemy bike;
			Enemy truck;
			Enemy tank;
            Enemy pickup;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(Enemies, solder, bike, truck, tank, pickup)
		} enemies;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, guns, enemies)
	} params;
private:
	ParamsManager();
};
