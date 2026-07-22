#pragma once
#include "gui/layered_icon.h"
#include "guns/building.h"
#include <functional>
#include <memory>
#include <array>

class BuildingButton;

class BuildingPanel : public Widget {
public:
    BuildingPanel(Widget* ui);
    void update(int player_coins);
    enum BuildResult {
        NO_SELECTED_BUILDING_BUTTON,
        NO_TILE_UNDER_MOUSE,
        INVALID_TILE_TYPE,
        SUCCESS
    };
    BuildResult build_if_allowed(const sf::Vector2f& mouse_pos);
    void unselect();
    void unselect(BuildingButton* button);
    void select(BuildingButton* button);
    void draw_building_plan(sf::RenderWindow& window, int x_id, int y_id);
    bool is_seleted() { return m_selected_button != nullptr; }
    Query on_event(Widget::EventContext context) override;
    Widget* ui;
private:
    BuildingButton* m_selected_button = nullptr;
    std::array<BuildingButton*, 8> m_buttons;
};

class BuildingButton : public LayeredIcon, private HoverableClickableWidget {
    friend class BuildingPanel;
public:
    using BuildingCreator = std::function<std::unique_ptr<IBuilding>(int x_id, int y_id)>;
    enum class TileRestrictions {
        NoRoads,
        RoadOnly
    };

    BuildingButton(const BuildingCreator& creator, BuildingType type, TileRestrictions restrictions, int cost, float radius, TextureID icon);
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

class MinigunBuildingButton : public BuildingButton {
public:
    MinigunBuildingButton();
};

class MineBuildingButton : public BuildingButton {
public:
    MineBuildingButton();
};

class SpikesBuildingButton : public BuildingButton {
public:
    SpikesBuildingButton();
    void draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) override;
};

class HedgehogBuildingButton : public BuildingButton {
public:
    HedgehogBuildingButton();
};

class AntitankBuildingButton : public BuildingButton {
public:
    AntitankBuildingButton();
    void draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) override;
};

class TwinGunBuildingButton : public BuildingButton {
public:
    TwinGunBuildingButton();
};


class RadarBuildingButton : public BuildingButton {
public:
    RadarBuildingButton();
};

class RadioMastBuildingButton : public BuildingButton {
public:
    RadioMastBuildingButton();
    void draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) override;
};
