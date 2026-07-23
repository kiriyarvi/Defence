#include <guns/building.h>

#pragma once


class BuildingCostComputer : public IBuildingVisitor {
public:
    void visit(MiniGun& minigun)override;
    void visit(Spikes& spikes) override;
    void visit(Hedgehog& headgehogs) override;
    void visit(AntitankGun& antitank_gun) override;
    void visit(TwinGun& twingun) override;
    void visit(Mine& mine) override;
    void visit(Radar& radar)override;
    void visit(RadioMast& radio_tower) override;
    int cost; //< вычисленное значение
};
