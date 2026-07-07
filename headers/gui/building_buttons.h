#pragma once

#include "gui/icon_button.h"
#include "tile_map.h"
#include <TGUI/TGUI.hpp>

#include <functional>
#include <memory>

class NBuildingButton;

class BuildingPanel : public Widget {
public:
    BuildingPanel(Widget* ui);
    void update(int player_coins);
    void build_if_allowed(const sf::Vector2f& mouse_pos);
    void unselect();
    void unselect(NBuildingButton* button);
    void select(NBuildingButton* button);
    void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id);
    Widget* ui;
private:
    NBuildingButton* m_selected_button = nullptr;
};

class NBuildingButton : public LayeredIcon {
    friend class BuildingPanel;
public:
    using BuildingCreator = std::function<std::unique_ptr<IBuilding>(int x_id, int y_id)>;
    enum class TileRestrictions {
        NoRoads,
        RoadOnly
    };

    NBuildingButton(const BuildingCreator& creator, BuildingType type, TileRestrictions restrictions, int cost, float radius, TextureID icon);
    template <typename T>
    static BuildingCreator make_creator() {
        return [](int x_id, int y_id) {
            return std::make_unique<T>(x_id, y_id);
        };
    }

    void update(int current_coins_count);
    void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id);
    virtual void draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed);
private:

    enum class State {
        ACTIVE,
        SELECTED,
        UNDISCOVERED,
        NOT_ENOUGTH_MONEY
    } m_state;
    void set_state(State state);
    bool is_cell_allowed(int x_id, int y_id);
    BuildingCreator m_creator;
    TileRestrictions m_restrictions;
    TextureID m_icon;
    Widget* m_tooltip;
protected:
    float m_radius;
    int m_cost;
    BuildingType m_type;
};

class NMinigunBuildingButton : public NBuildingButton {
public:
    NMinigunBuildingButton();
};

class NMineBuildingButton : public NBuildingButton {
public:
    NMineBuildingButton();
};

class NSpikesBuildingButton : public NBuildingButton {
public:
    NSpikesBuildingButton();
    void draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) override;
};

class NHedgehogBuildingButton : public NBuildingButton {
public:
    NHedgehogBuildingButton();
};

class NAntitankBuildingButton : public NBuildingButton {
public:
    NAntitankBuildingButton();
    void draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) override;
};

class NTwinGunBuildingButton : public NBuildingButton {
public:
    NTwinGunBuildingButton();
};


class NRadarBuildingButton : public NBuildingButton {
public:
    NRadarBuildingButton();
};

class NRadioMastBuildingButton : public NBuildingButton {
public:
    NRadioMastBuildingButton();
};




class GameState;

class BuildingButton: public IconButton {
public:
	using BuildingCreator = std::function<std::unique_ptr<IBuilding>(int x_id, int y_id)>;

	enum class TileRestrictions {
		NoRoads,
		RoadOnly
	} restrictions;

	BuildingButton(TextureID gun_icon, GameState& game_state, const BuildingCreator& creator, TileRestrictions restrictions, int cost, float radius, BuildingType type, const std::string& name);
	BuildingButton(BuildingButton&& btn);
	BuildingCreator creator;
	template <typename T>
	static BuildingCreator make_creator() {
		return [](int x_id, int y_id) {
			return std::make_unique<T>(x_id, y_id);
		};
	}
	bool is_cell_allowed(int x_id, int y_id);
	virtual void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id);
	void unselect();
	void coins_update(int current_coins_count);
    void defeat_event();
    void show_info_content();
	int cost;
private:
	void connect();
protected:
	void draw_radius(sf::RenderWindow& window, int x_id, int y_id);
    void lock_button(bool lock);
protected:
	GameState& m_game_state;
	float m_radius;
    BuildingType m_type;
    std::string m_name;
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

class RadarBuildingButton : public BuildingButton {
public:
    RadarBuildingButton(GameState& game_state);
    void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) override;
};

class RadioTowerBuildingButton : public BuildingButton {
public:
    RadioTowerBuildingButton(GameState& game_state);
    void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) override;
};
