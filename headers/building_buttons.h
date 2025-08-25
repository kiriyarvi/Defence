#pragma once

#include "tile_map.h"

#include <TGUI/TGUI.hpp>

#include <functional>
#include <memory>


class GameState;

class BuildingButton {
public:
	using BuildingCreator = std::function<std::unique_ptr<IBuilding>()>;

	enum class TileRestrictions {
		NoRoads,
		RoadOnly
	} restrictions;

	BuildingButton(TileTexture gun_icon, GameState& game_state, const BuildingCreator& creator, TileRestrictions restrictions, int cost, float radius);
	BuildingButton(BuildingButton&& btn);
	BuildingCreator creator;
	template <typename T>
	static BuildingCreator make_creator() {
		return []() {
			return std::make_unique<T>();
		};
	}
	bool is_cell_allowed(int x_id, int y_id);
	virtual void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id);
	void disable_selection();
	void coins_update(int current_coins_count);
public:
	tgui::BitmapButton::Ptr button;
	int cost;
	bool disabled = false;
private:
	void connect();
protected:
	void draw_radius(sf::RenderWindow& window, int x_id, int y_id);
protected:
	TileTexture m_gun_icon;
	GameState& m_game_state;
	float m_radius;
	tgui::Label::Ptr m_tooltip;
};


class MinigunBuildingButton : public BuildingButton {
public:
	MinigunBuildingButton(GameState& game_state);
};

class MineBuildingButton : public BuildingButton {
public:
	MineBuildingButton(GameState& game_state);
};

class TwinGunBuildingButton : public BuildingButton {
public:
	TwinGunBuildingButton(GameState& game_state);
	void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) override;
};

class AntitankGunBuildingButton : public BuildingButton {
public:
	AntitankGunBuildingButton(GameState& game_state);
	void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) override;
};

class SpikesBuildingButton : public BuildingButton {
public:
	SpikesBuildingButton(GameState& game_state);
	void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) override;
};

class HedgeBuildingButton : public BuildingButton {
public:
	HedgeBuildingButton(GameState& game_state);
	void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) override;
};