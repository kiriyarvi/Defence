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
				float max_damage;
                float min_damage;
                float splash_radius;
                float max_damage_zone_radius;
				float cooldown;
				float interleaved_cooldown;
                int armor_penetration_level;
                int cost;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(TwinGun, radius, max_damage, min_damage, splash_radius, max_damage_zone_radius, cooldown, interleaved_cooldown, armor_penetration_level, cost)
			} twingun;
			struct Minigun {
				float radius;
				float min_rotation_speed;
				float max_rotation_speed;
				float critical_temperature;
				int cost;
				struct PenetrationUpgrade {
					int min_armor_penetration_level;
					int max_armor_penetration_level;
                    int min_damage;
                    int max_damage;
					NLOHMANN_DEFINE_TYPE_INTRUSIVE(PenetrationUpgrade, min_armor_penetration_level, max_armor_penetration_level, min_damage, max_damage)
				};
				std::vector<PenetrationUpgrade> penetration_upgrades;
                struct CoolingUpgrade {
                    float critical_temperature_work_duration;
                    float cooling_time;
                    float cooldown_duration;
                    NLOHMANN_DEFINE_TYPE_INTRUSIVE(CoolingUpgrade, critical_temperature_work_duration, cooling_time, cooldown_duration)
                };
                std::vector<CoolingUpgrade> cooling_upgrades;
                struct LubricantUpgrade {
                    float heating_time;
                    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LubricantUpgrade, heating_time)
                };
                std::vector<LubricantUpgrade> lubricant_upgrades;

                float rotation_speed;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Minigun,
					radius, min_rotation_speed, max_rotation_speed, critical_temperature,
					cost, penetration_upgrades, cooling_upgrades, lubricant_upgrades, rotation_speed
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
            Enemy BTR;
            Enemy CruiserI;
            Enemy smoke_truck;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(Enemies, solder, bike, truck, tank, pickup, BTR, CruiserI, smoke_truck)
		} enemies;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, guns, enemies)
	} params;
private:
	ParamsManager();
};
