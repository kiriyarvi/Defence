#pragma once
#include <SFML/Graphics.hpp>

enum class BuildingType {
    Minigun,
    Spikes,
    Mine,
    Hedgehogs,
    AntitankGun,
    TwinGun,
    Radar,
    RadioMast
};

class MiniGun;
class Spikes;
class Hedgehog;
class AntitankGun;
class TwinGun;
class Mine;
class Radar;
class RadioMast;

class IBuildingVisitor {
public:
    virtual void visit(MiniGun& minigun) = 0;
    virtual void visit(Spikes& spikes) = 0;
    virtual void visit(Hedgehog& headgehogs) = 0;
    virtual void visit(AntitankGun& antitank_gun) = 0;
    virtual void visit(TwinGun& twingun) = 0;
    virtual void visit(Mine& mine) = 0;
    virtual void visit(Radar& radar) = 0;
    virtual void visit(RadioMast& radio_tower) = 0;
};


enum class LanguageCase {
    NOMINATIVE, //< именительный
    GENETIVE, //< родительный
};

std::string to_string(BuildingType type, LanguageCase lan_case = LanguageCase::NOMINATIVE);


#define ACCEPT(Type) \
void accept(IBuildingVisitor& visitor) override { visitor.visit(*this); }

class IBuilding {
public:
    IBuilding(int x_id, int y_id, BuildingType type);
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void draw_effects(sf::RenderWindow& window) = 0;
    virtual void logic(double dtime) = 0;
    virtual bool is_destroyed() { return false; }
    virtual void accept(IBuildingVisitor& visitor) = 0;
    virtual ~IBuilding() = default;
    int x_id;
    int y_id;
    BuildingType type;
};
