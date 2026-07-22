#include "guns/building.h"
#include <unordered_map>

IBuilding::IBuilding(int x_id, int y_id, BuildingType type) : x_id{ x_id }, y_id{ y_id }, type{type} {}


std::string to_string(BuildingType type, LanguageCase lan_case) {
	static std::unordered_map<BuildingType, std::string> nom{
		{BuildingType::AntitankGun, "Противотанковая пушка"},
		{BuildingType::Hedgehogs, "Противотанковые ежи"},
		{BuildingType::Mine, "Мина"},
		{BuildingType::Minigun, "Пулемет"},
		{BuildingType::Spikes, "Шипы"},
		{BuildingType::TwinGun, "Сдвоенная пушка"},
		{BuildingType::Radar, "Радар"},
		{BuildingType::RadioMast, "Радиовышка"}
	};
    static std::unordered_map<BuildingType, std::string> gen{
		{BuildingType::AntitankGun, "Противотанковой пушки"},
		{BuildingType::Hedgehogs, "Противотанковых ежей"},
		{BuildingType::Mine, "Мины"},
		{BuildingType::Minigun, "Пулемета"},
		{BuildingType::Spikes, "Шипов"},
		{BuildingType::TwinGun, "Сдвоенной пушки"},
		{BuildingType::Radar, "Радара"},
		{BuildingType::RadioMast, "Радиовышки"}
	};
	switch (lan_case) {
	case LanguageCase::NOMINATIVE:
		return nom[type];
	case LanguageCase::GENETIVE:
		return gen[type];
	}
}
