#include "guns/building_cost_computer.h"
#include <guns/mine.h>
#include <guns/minigun.h>
#include <guns/antitank_gun.h>
#include <guns/twin_gun.h>
#include <guns/hedgehog.h>
#include <guns/spikes.h>
#include <guns/radio_mast.h>
#include <guns/radar.h>


void BuildingCostComputer::visit(MiniGun& minigun) {
    cost = 0;
    auto& params = ParamsManager::Instance().params.guns.minigun;
    for (size_t i = 1; i <= minigun.m_penetration_upgrade; ++i)
        cost += params.penetration_upgrades[i].cost;
    for (size_t i = 1; i <= minigun.m_cooling_upgrade; ++i)
        cost += params.cooling_upgrades[i].cost;
    for (size_t i = 1; i <= minigun.m_lubricant_upgrade; ++i)
        cost += params.lubricant_upgrades[i].cost;
    cost += params.cost;
}

void BuildingCostComputer::visit(Spikes& spikes) {
    cost = 0;
    auto& params = ParamsManager::Instance().params.guns.spikes;
    cost += params.cost * (spikes.get_health() / static_cast<float>(params.health)); //возвращаем ровно столько, сколько стоят оставшиеся использования
}
void BuildingCostComputer::visit(Hedgehog& headgehogs) {
    cost = 0;
    auto& params = ParamsManager::Instance().params.guns.hedgehog;
    cost += params.cost * (headgehogs.get_health() / static_cast<float>(params.health)); //возвращаем ровно столько, сколько стоят оставшиеся использования

}
void BuildingCostComputer::visit(AntitankGun& antitank_gun) {
    auto& params = ParamsManager::Instance().params.guns.antitank;
    cost = params.cost;
}

void BuildingCostComputer::visit(TwinGun& twingun) {
    auto& params = ParamsManager::Instance().params.guns.twingun;
    cost = params.cost;
}
void BuildingCostComputer::visit(Mine& mine) {
    cost = 0; //нельзя продавать
}

void BuildingCostComputer::visit(Radar& radar){
    auto& params = ParamsManager::Instance().params.guns.radar;
    cost = 0;
    for (size_t i = 1; i <= radar.uncovering_level_upgrade; ++i)
        cost += params.uncovering_level_upgrades[i].cost;
    for (size_t i = 1; i <= radar.uncovering_speed_upgrade; ++i)
        cost += params.uncovering_speed_upgrades[i].cost;
    for (size_t i = 1; i <= radar.radius_upgrade; ++i)
        cost += params.radius_upgrades[i].cost;
    if (radar.long_distance_communication_upgrade)
        cost += params.long_distance_communication_upgrade_cost;
    cost += params.cost;

}
void BuildingCostComputer::visit(RadioMast& radio_tower) {
    auto& params = ParamsManager::Instance().params.guns.radio_tower;
    cost = params.cost;
}
