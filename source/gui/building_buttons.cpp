#include "gui/building_buttons.h"
#include "game_state.h"

#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "guns/minigun.h"
#include "guns/mine.h"
#include "guns/spikes.h"
#include "guns/hedgehog.h"
#include "guns/radar.h"
#include "guns/radio_mast.h"
#include "shader_manager.h"
#include "achievement_system.h"
#include "gui/info_panel.h"
#include "net_manager.h"


BuildingPanel::BuildingPanel(Widget* ui) : ui{ui}, Widget(ui) {
    Widget* b1 = add(std::make_unique<MinigunBuildingButton>());
    Widget* b2 = add(std::make_unique<MineBuildingButton>());
    Widget* b3 = add(std::make_unique<SpikesBuildingButton>());
    Widget* b4 = add(std::make_unique<HedgehogBuildingButton>());
    Widget* b5 = add(std::make_unique<AntitankBuildingButton>());
    Widget* b6 = add(std::make_unique<TwinGunBuildingButton>());
    Widget* b7 = add(std::make_unique<RadarBuildingButton>());
    Widget* b8 = add(std::make_unique<RadioMastBuildingButton>());
    DEBUG_TAG(b1, "MinigunBuildingButton")
    DEBUG_TAG(b2, "MineBuildingButton")
    DEBUG_TAG(b3, "SpikesBuildingButton")
    DEBUG_TAG(b4, "HedgehogBuildingButton")
    DEBUG_TAG(b5, "AntitankBuildingButton")
    DEBUG_TAG(b6, "TwinGunBuildingButton")
    DEBUG_TAG(b7, "RadarBuildingButton")
    DEBUG_TAG(b8, "RadioMastBuildingButton")


    auto prev_it = m_children.end();
    for (auto button_it = m_children.begin(); button_it != m_children.end(); ++button_it) {
        Widget* button = button_it->get();
        button->add_rule(Property::SIZE, [ui](Widget::Layout& layout) {
            layout.height = ui->layout.height * 0.1;
            layout.width = layout.height;
        }, { { ui, Property::HEIGHT } });
        Widget* prev = nullptr;
        if (prev_it == m_children.end()) {
            prev_it = button_it;
            continue;
        }
        prev = prev_it->get();
        Widget* button_panel = this;
        button->add_rule(Property::POSITION, [button_panel, prev](Widget::Layout& layout) {
            if (!prev)
                return;
            float line_width = prev->layout.x + prev->layout.width + layout.width;
            if (line_width >= button_panel->layout.width) {
                layout.x = 0;
                layout.y = prev->layout.y + prev->layout.height;
            }
            else {
                layout.x = prev->layout.x + prev->layout.width;
                layout.y = prev->layout.y;
            }
        }, { {button_panel, Property::WIDTH}, { prev, Property::LAYOUT }, { this, Property::WIDTH } });
        prev_it = button_it;
    }

    position_anchor(Anchor::LEFT | Anchor::BOTTOM, ui, Anchor::LEFT | Anchor::BOTTOM);
    add_rule(Property::WIDTH, [ui](Widget::Layout& layout) {
        layout.width = ui->layout.width * 0.7;
    }, { {ui, Property::WIDTH} });
    Widget* last_button = m_children.back().get();
    add_rule(Property::HEIGHT, [last_button](Widget::Layout& layout) {
        layout.height = last_button->layout.y + last_button->layout.height;
    }, { {last_button, Property::Y | Property::HEIGHT} });

}

void BuildingPanel::update(int player_coins) {
    for (auto& button : m_children)
        dynamic_cast<BuildingButton*>(button.get())->update(player_coins);
    if (m_selected_button && m_selected_button->m_state != BuildingButton::State::SELECTED)
        m_selected_button = nullptr;
}

void BuildingPanel::build_if_allowed(const sf::Vector2f& mouse_pos) {
    if (!m_selected_button)
        return;
    size_t N = TileMap::Instance().map.size();
    bool on_map = mouse_pos.x < N * 32 && mouse_pos.x >= 0 && mouse_pos.y < N * 32 && mouse_pos.y >= 0;
    sf::Vector2i cell_id(mouse_pos.x / 32, mouse_pos.y / 32);
    if (on_map && m_selected_button->is_cell_allowed(cell_id.x, cell_id.y)) {
        TileMap::Instance().map[cell_id.x][cell_id.y].building = m_selected_button->m_creator(cell_id.x, cell_id.y);
        GameState::Instance().player_coins_add(-m_selected_button->m_cost);
    }
}

void BuildingPanel::select(BuildingButton* button_to_select) {
    for (auto& button_ptr : m_children) {
        BuildingButton* button = dynamic_cast<BuildingButton*>(button_ptr.get());
        if (button == button_to_select) {
            button->set_state(BuildingButton::State::SELECTED);
            m_selected_button = button_to_select;
        }
        else if (button->m_state == BuildingButton::State::SELECTED)
            button->set_state(BuildingButton::State::ACTIVE);
    }
}

void BuildingPanel::unselect() {
    if (m_selected_button) {
        assert(m_selected_button->m_state == BuildingButton::State::SELECTED);
        m_selected_button->set_state(BuildingButton::State::ACTIVE);
    }
    m_selected_button = nullptr;
}

void BuildingPanel::unselect(BuildingButton* button) {
    if (m_selected_button == button) {
        m_selected_button->set_state(BuildingButton::State::ACTIVE);
        m_selected_button = nullptr;
    }
}

void BuildingPanel::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
    if (!m_selected_button)
        return;
    m_selected_button->draw_building_plan(window, x_id, y_id);
}

BuildingButton::BuildingButton(const BuildingCreator& creator, BuildingType type, TileRestrictions restrictions, int cost, float radius, TextureID icon):
    m_creator{ creator }, m_restrictions{ restrictions }, m_radius{ radius }, m_type{ type }, m_icon{ icon }, m_cost{ cost }
{
    set_state(State::UNDISCOVERED);
    on_pressed = [this](const glm::vec2& position_transform, const glm::vec2& mouse_pos, const sf::Event::MouseButtonEvent& event) {
        if (m_state == State::UNDISCOVERED || m_state == State::NOT_ENOUGTH_MONEY)
            return;
        if (event.button == sf::Mouse::Button::Left) {
            BuildingPanel* building_panel = dynamic_cast<BuildingPanel*>(m_parent);
            building_panel->select(this);
            return;
        }
    };
    on_hovered = [this](const glm::vec2& position_transform, const glm::vec2& mouse_pos) {
        //TODO здесь есть проблема. Допустим показывается tooltip и вдруг кнопка разблокировалась, а описание не поменялось
        BuildingPanel* building_panel = dynamic_cast<BuildingPanel*>(m_parent);
        Panel* panel = (Panel*)m_parent->add(Panel::create(sf::Color(50, 50, 50, 255), sf::Color::Black, 0));
        DEBUG_TAG(panel, "tooltip_panel")
        Label* label = (Label*)panel->add(Label::create(true));
        DEBUG_TAG(label, "tooltip_label")
        label->add_text(to_string(m_type) + "\n", sf::Color::White, sf::Text::Style::Bold);
        label->add_text("Стоимость: " + std::to_string(m_cost) + "\n", Label::gold_color);
        if (m_state == State::UNDISCOVERED)
            label->add_text("Закрыто: " + AchievementSystem::Instance().get_building_unlock_condition_description(m_type) + "\n", sf::Color::Red);
        label->add_text("Откройте справку, для получения подробностей", sf::Color::White, sf::Text::Style::Italic);
        
        panel->size_include(label);
        panel->position_tooltip(Anchor::BOTTOM | Anchor::LEFT);
        panel->receive_mouse_events = false;
        m_tooltip = panel;
    };

    on_unhovered = [this](const glm::vec2& position_transform, const glm::vec2& mouse_pos) {
        m_parent->delete_widget(m_tooltip);
    };
}

void BuildingButton::update(int current_coins_count) {
    bool unlocked = AchievementSystem::Instance().is_unlocked(m_type);
    bool enougth_money = m_cost <= current_coins_count;

    if (m_state == State::SELECTED) {
        if (!enougth_money)
            set_state(State::NOT_ENOUGTH_MONEY);
        return;
    }

    if (!unlocked)
        set_state(State::UNDISCOVERED);
    else {
        if (enougth_money)
            set_state(State::ACTIVE);
        else
            set_state(State::NOT_ENOUGTH_MONEY);
    }

}


void BuildingButton::set_state(State state) {
    if (m_state == state)
        return;
    switch (state) {
    case BuildingButton::State::ACTIVE:
        grayscale = false;
        layers = { TextureID::ButtonBackground, m_icon };
        break;
    case BuildingButton::State::SELECTED:
        grayscale = false;
        layers = { TextureID::ButtonClickedBackground, m_icon };
        break;
    case BuildingButton::State::UNDISCOVERED:
        grayscale = true;
        layers = { TextureID::ButtonBackground, m_icon, TextureID::Locked };
        break;
    case BuildingButton::State::NOT_ENOUGTH_MONEY:
        grayscale = true;
        layers = { TextureID::ButtonBackground, m_icon };
        break;
    default:
        break;
    }
    m_state = state;
}


bool BuildingButton::is_cell_allowed(int x_id, int y_id) {
    auto& tile = TileMap::Instance().map[x_id][y_id];
    if (tile.building)
        return false;
    int roads_count = std::count(tile.roads.begin(), tile.roads.end(), true);
    if (m_restrictions == TileRestrictions::RoadOnly)
        return roads_count;
    else if (m_restrictions == TileRestrictions::NoRoads)
        return !roads_count;
    return true;
}

void BuildingButton::draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) {
    sf::Texture& texture = TextureManager::Instance().textures[m_icon];
    sf::Sprite building(texture);
    building.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    building.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
    building.rotate(180);
    if (!allowed) building.setColor(sf::Color(255, 0, 0));
    window.draw(building);
}

void BuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
    bool allowed = is_cell_allowed(x_id, y_id);
    if (allowed && m_radius != 0.0) {
        sf::CircleShape circle(m_radius * 32, 30);
        circle.setFillColor(sf::Color(0, 255, 0, 40));
        circle.setOutlineThickness(2);
        circle.setOutlineColor(sf::Color(0, 255, 0, 140));
        circle.setPosition(x_id * 32 + 16, y_id * 32 + 16);
        circle.setOrigin(m_radius * 32, m_radius * 32);
        window.draw(circle);
    }
    draw_building(window, x_id, y_id, allowed);
}

MinigunBuildingButton::MinigunBuildingButton():
    BuildingButton(
        BuildingButton::make_creator<MiniGun>(),
        BuildingType::Minigun,
        BuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.minigun.cost,
        ParamsManager::Instance().params.guns.minigun.radius,
        TextureID::MinigunIcon
        )
{}

MineBuildingButton::MineBuildingButton() :
    BuildingButton(
        BuildingButton::make_creator<Mine>(),
        BuildingType::Mine,
        BuildingButton::TileRestrictions::RoadOnly,
        ParamsManager::Instance().params.guns.mine.cost,
        ParamsManager::Instance().params.guns.mine.damage_radius,
        TextureID::Mine
    ) {}


SpikesBuildingButton::SpikesBuildingButton():
    BuildingButton(
        BuildingButton::make_creator<Spikes>(),
        BuildingType::Spikes,
        BuildingButton::TileRestrictions::RoadOnly,
        ParamsManager::Instance().params.guns.spikes.cost,
        0,
        TextureID::SpikesIcon
    ) {}

void SpikesBuildingButton::draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) {
    sf::Sprite spikes = Spikes::get_sprite_for_tile(x_id, y_id);
    spikes.setOrigin(16, 16);
    spikes.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    if (!allowed) spikes.setColor(sf::Color(255, 0, 0));
    window.draw(spikes);
}

HedgehogBuildingButton::HedgehogBuildingButton():
    BuildingButton(
        BuildingButton::make_creator<Hedgehog>(),
        BuildingType::Hedgehogs,
        BuildingButton::TileRestrictions::RoadOnly,
        ParamsManager::Instance().params.guns.hedgehog.cost,
        0,
        TextureID::Hedgehog
    ) {}

AntitankBuildingButton::AntitankBuildingButton():
    BuildingButton(
        BuildingButton::make_creator<AntitankGun>(),
        BuildingType::AntitankGun,
        BuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.antitank.cost,
        ParamsManager::Instance().params.guns.antitank.radius,
        TextureID::AntitankGunIcon
    ) {}

void AntitankBuildingButton::draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) {
    sf::Sprite base(TextureManager::Instance().textures[TextureID::GunBase]);
    base.setPosition(x_id * 32, y_id * 32);
    if (!allowed) base.setColor(sf::Color(255, 0, 0));
    window.draw(base);
    sf::Sprite gun(TextureManager::Instance().textures[TextureID::AntitankGunConstructed]);
    gun.setOrigin(9, 16);
    gun.rotate(180);
    gun.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    if (!allowed) gun.setColor(sf::Color(255, 0, 0));
    window.draw(gun);
}


TwinGunBuildingButton::TwinGunBuildingButton():
    BuildingButton(
        BuildingButton::make_creator<TwinGun>(),
        BuildingType::TwinGun,
        BuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.twingun.cost,
        ParamsManager::Instance().params.guns.twingun.radius,
        TextureID::TwingunIcon
    ) {}

RadarBuildingButton::RadarBuildingButton():
    BuildingButton(
        [](int x_id, int y_id) {
            auto radar = std::make_unique<Radar>(x_id, y_id);
            NetManager::Instance().new_radar(x_id, y_id, radar.get());
            return radar;
        },
        BuildingType::Radar,
        BuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.radar.cost,
        ParamsManager::Instance().params.guns.radar.radius_upgrades[0].radius,
        TextureID::RadarIcon
    ) {}

RadioMastBuildingButton::RadioMastBuildingButton():
    BuildingButton(
        [](int x_id, int y_id) {
            auto radio_tower = std::make_unique<RadioMast>(x_id, y_id);
            NetManager::Instance().new_radio_tower(x_id, y_id);
            return radio_tower;
        },
        BuildingType::RadioMast,
        BuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.radio_tower.cost,
        ParamsManager::Instance().params.guns.radio_tower.radius,
        TextureID::RadioMast
    ) {}
