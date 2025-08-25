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
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Antitank, radius, damage, cooldown)
			} antitank;
			struct TwinGun {
				float radius;
				float damage_per_barrel;
				float cooldown;
				float interleaved_cooldown;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(TwinGun, radius, damage_per_barrel, cooldown, interleaved_cooldown)
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
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Minigun,
					radius, heating_time, cooling_time,
					min_rotation_speed, max_rotation_speed,
					min_damage, max_damage,
					cooldown_duration, critical_temperature,
					critical_temperature_work_duration
				)
			} minigun;
			struct Mine {
				float damage_radius;
				float activation_radius;
				float min_damage;
				float max_damage;
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mine, damage_radius, activation_radius, min_damage, max_damage)
			} mine;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(Guns, antitank, twingun, minigun, mine)
		} guns;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, guns)
	} params;
private:
	ParamsManager();
};