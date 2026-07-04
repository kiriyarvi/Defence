#include "gui/building_buttons.h"
#include "game_state.h"

#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "guns/minigun.h"
#include "guns/mine.h"
#include "guns/spikes.h"
#include "guns/hedgehog.h"
#include "guns/radar.h"
#include "guns/radio_tower.h"
#include "shader_manager.h"
#include "achievement_system.h"
#include "gui/info_panel.h"
#include "net_manager.h"

NBuildingButton::NBuildingButton(std::vector<NBuildingButton*>* buttons, Widget* ui, const BuildingCreator& creator, BuildingType type, TileRestrictions restrictions, int cost, float radius, TextureID icon):
    m_buttons{ buttons }, m_ui{ui}, m_creator {
    creator
}, m_restrictions{ restrictions }, m_radius{ radius }, m_type{ type }, m_icon{ icon }, m_cost{ cost }
{
    m_delegate_all_events = false;
    set_state(State::UNDISCOVERED);
    on_event = [this](const glm::vec2& position_transform, const sf::Event& event) -> bool {
        if (m_state == State::UNDISCOVERED || m_state == State::NOT_ENOUGTH_MONEY)
            return true;
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Button::Left) {
            set_state(NBuildingButton::State::SELECTED); //select this
            for (auto btn : *m_buttons) { //unselect others
                if (btn != this)
                    btn->unselect();
            }
            return true;
        }
    };
    on_hovered = [this]() {
        Panel* panel = (Panel*)m_ui->add(Panel::create());
        panel->set_border(0);
        Label* label = (Label*)panel->add(Label::create(true));
        label->add_text(to_string(m_type) + "\n", sf::Color::White, sf::Text::Style::Bold);
        label->add_text("Стоимость: " + std::to_string(m_cost) + "\n", Label::gold_color);
        if (m_state == State::UNDISCOVERED)
            label->add_text("Закрыто: " + AchievementSystem::Instance().get_building_unlock_condition_description(m_type), sf::Color::Red);
        label->add_text("Откройте справку, для получения подробностей", sf::Color::White, sf::Text::Style::Italic);

        panel->size_inherited(label);
        panel->position_tooltip(m_ui, Anchor::BOTTOM | Anchor::LEFT);
        m_tooltip = panel;
    };

    on_unhovered = [this]() {
        m_ui->delete_widget(m_tooltip);
    };
}

void NBuildingButton::coins_update(int current_coins_count) {
    if (m_state == State::NOT_ENOUGTH_MONEY && m_cost <= current_coins_count)
        set_state(State::ACTIVE);
    if ((m_state == State::ACTIVE || m_state == State::SELECTED) && m_cost > current_coins_count)
        set_state(State::NOT_ENOUGTH_MONEY);
}

void NBuildingButton::achievement_event(int current_coins_count) {
    if (m_state == State::UNDISCOVERED && AchievementSystem::Instance().is_unlocked(m_type)) {
        if (m_cost <= current_coins_count)
            set_state(State::ACTIVE);
        else
            set_state(State::NOT_ENOUGTH_MONEY);
    }
}

void NBuildingButton::unselect() {
    if (m_state == NBuildingButton::State::SELECTED)
        set_state(NBuildingButton::State::ACTIVE);
}

void NBuildingButton::set_state(State state) {
    switch (state) {
    case NBuildingButton::State::ACTIVE:
        grayscale = false;
        layers = { TextureID::ButtonBackground, m_icon };
        break;
    case NBuildingButton::State::SELECTED:
        grayscale = false;
        layers = { TextureID::ButtonClickedBackground, m_icon };
        break;
    case NBuildingButton::State::UNDISCOVERED:
        grayscale = true;
        layers = { TextureID::ButtonBackground, m_icon, TextureID::Locked };
        break;
    case NBuildingButton::State::NOT_ENOUGTH_MONEY:
        grayscale = true;
        layers = { TextureID::ButtonBackground, m_icon };
        break;
    default:
        break;
    }
}


bool NBuildingButton::is_cell_allowed(int x_id, int y_id) {
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

void NBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
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


NMinigunBuildingButton::NMinigunBuildingButton(std::vector<NBuildingButton*>* buttons, Widget* ui):
    NBuildingButton(
        buttons,
        ui,
        NBuildingButton::make_creator<MiniGun>(),
        BuildingType::Minigun,
        NBuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.minigun.cost,
        ParamsManager::Instance().params.guns.minigun.radius,
        TextureID::MinigunIcon
        )
{}

void NMinigunBuildingButton::draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) {
    sf::Sprite minigun(TextureManager::Instance().textures[TextureID::MinigunIcon]);
    minigun.setPosition(x_id * 32, y_id * 32);
    if (!allowed) minigun.setColor(sf::Color(255, 0, 0));
    window.draw(minigun);
}

BuildingButton::BuildingButton(TextureID gun_icon, GameState& game_state, const BuildingCreator& creator, TileRestrictions restrictions, int cost, float radius, BuildingType type, const std::string& name)
    : creator{ creator }, restrictions{ restrictions }, m_game_state{ game_state }, cost{ cost }, m_radius(radius), m_type{ type }, m_name{name}, IconButton(gun_icon, TextureID::ButtonBackground, TextureID::ButtonClickedBackground) {
    connect();
    if (AchievementSystem::Instance().is_unlocked(m_type))
        lock_button(false);
}

void BuildingButton::connect() {
	m_button->onPress.disconnectAll();
    m_button->onPress.connect([&]() {
        for (auto& button : m_game_state.m_building_buttons) {
            if (button.get() != this)
                button->unselect();
        }
        if (m_state == State::Active) {
            set_state(State::Selected);
            m_game_state.m_current_building_construction = this;
        }
	});
    m_group->onMouseEnter.disconnectAll();
    m_group->onMouseEnter([&]() {
        show_info_content();
    });
    m_group->onMouseLeave.disconnectAll();
    m_group->onMouseLeave([&]() {
        m_game_state.set_tooltip_content("");
    });
   
}

void BuildingButton::show_info_content() {
    std::string content =
        "<b>" + m_name + "</b>\n"
        "<color=#ffd303>Стоимость: " + std::to_string(cost) + "</color>\n"
        + (m_state == State::Locked ? "<color=#ff0000>Закрыто: " + AchievementSystem::Instance().get_building_unlock_condition_description(m_type) + "</color>\n" : "") +
        "<i>Откройте справку, для получения подробностей</i>";
    m_game_state.set_tooltip_content(content);
}

BuildingButton::BuildingButton(BuildingButton&& btn) :
    creator{ btn.creator }, restrictions{ btn.restrictions }, m_game_state{ btn.m_game_state }, cost{ btn.cost }, m_radius{ btn.m_radius }, m_type{ btn.m_type }, m_name{btn.m_name}, IconButton(std::move(btn))
{
	connect(); // нужно переназначать, поскольку используем this в lambda-функции.
}

void BuildingButton::coins_update(int current_coins_count) {
    if (m_state == State::Locked)
        return;
	bool disabled = current_coins_count < cost;
    if (disabled && m_state != State::Disabled) {
        set_state(State::Disabled);
    }
    if (!disabled && m_state == State::Disabled) {
        set_state(State::Active);
    }
}


bool BuildingButton::is_cell_allowed(int x_id, int y_id) {
	auto& tile = TileMap::Instance().map[x_id][y_id];
	if (tile.building)
		return false;
	int roads_count = std::count(tile.roads.begin(), tile.roads.end(), true);
	if (restrictions == TileRestrictions::RoadOnly)
		return roads_count;
	else if (restrictions == TileRestrictions::NoRoads) {
		return !roads_count;
	}
	return true;
}

void BuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
	bool allowed = is_cell_allowed(x_id, y_id);
	if (allowed) {
		draw_radius(window, x_id, y_id);
	}
	auto& gun_texture = TextureManager::Instance().textures[m_icon];
	sf::Sprite gun(gun_texture);
	gun.setOrigin(gun_texture.getSize().x / 2.f, gun_texture.getSize().y / 2.f);
	gun.rotate(180);
	gun.setPosition(x_id * 32 + 16, y_id * 32 + 16);
	if (!allowed) gun.setColor(sf::Color(255, 0, 0));
	window.draw(gun);
}

void BuildingButton::draw_radius(sf::RenderWindow& window, int x_id, int y_id) {
	sf::CircleShape circle(m_radius * 32, 30);
	circle.setFillColor(sf::Color(0, 255, 0, 40));
	circle.setOutlineThickness(2);
	circle.setOutlineColor(sf::Color(0, 255, 0, 140));
	circle.setPosition(x_id * 32 + 16, y_id * 32 + 16);
	circle.setOrigin(m_radius * 32, m_radius * 32);
	window.draw(circle);
}



void BuildingButton::unselect() {
    if (m_state == State::Selected) {
        set_state(State::Active);
    }
}

void BuildingButton::defeat_event() {
    if (m_state == State::Locked)
        lock_button(!AchievementSystem::Instance().is_unlocked(m_type));
}

void BuildingButton::lock_button(bool l) {
    if (l)
        set_state(State::Locked);
    else {
        set_state(State::Active);
        coins_update(m_game_state.m_player_coins);
    }
}


MinigunBuildingButton::MinigunBuildingButton(GameState& game_state):
	BuildingButton(
        TextureID::MinigunIcon,
		game_state,
		make_creator<MiniGun>(),
		TileRestrictions::NoRoads,
		ParamsManager::Instance().params.guns.minigun.cost,
		ParamsManager::Instance().params.guns.minigun.radius,
        BuildingType::Minigun,
        "Пулемёт"
	)
{}


MineBuildingButton::MineBuildingButton(GameState& game_state):
	BuildingButton(
        TextureID::Mine,
		game_state,
		make_creator<Mine>(),
		TileRestrictions::RoadOnly,
		ParamsManager::Instance().params.guns.mine.cost,
		ParamsManager::Instance().params.guns.mine.damage_radius,
        BuildingType::Mine,
        "Мина"
	) 
{}

TwinGunBuildingButton::TwinGunBuildingButton(GameState& game_state) :
	BuildingButton(
        TextureID::TwingunIcon,
		game_state,
		make_creator<TwinGun>(),
		TileRestrictions::NoRoads,
		ParamsManager::Instance().params.guns.twingun.cost,
		ParamsManager::Instance().params.guns.twingun.radius,
        BuildingType::TwinGun,
        "Двуствольная пушка"
	)
{}

void TwinGunBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
	bool allowed = is_cell_allowed(x_id, y_id);
	if (allowed) {
		draw_radius(window, x_id, y_id);
	}
	sf::Sprite base(TextureManager::Instance().textures[TextureID::GunBase]);
	base.setPosition(x_id * 32, y_id * 32);
	if (!allowed) base.setColor(sf::Color(255, 0, 0));
	window.draw(base);

	sf::Sprite gun(TextureManager::Instance().textures[TextureID::TwingunConstructed]);
	gun.setOrigin(16, 15.5);
	gun.rotate(180);
	gun.setPosition(x_id * 32 + 16, y_id * 32 + 16);
	if (!allowed) gun.setColor(sf::Color(255, 0, 0));
	window.draw(gun);
}

AntitankGunBuildingButton::AntitankGunBuildingButton(GameState& game_state) :
	BuildingButton(
		TextureID::AntitankGunIcon,
		game_state,
		make_creator<AntitankGun>(),
		TileRestrictions::NoRoads,
		ParamsManager::Instance().params.guns.antitank.cost,
		ParamsManager::Instance().params.guns.antitank.radius,
        BuildingType::AntitankGun,
        "Противотанковая пушка"
	)
{
}

void AntitankGunBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
	bool allowed = is_cell_allowed(x_id, y_id);
	if (allowed) {
		draw_radius(window, x_id, y_id);
	}
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

SpikesBuildingButton::SpikesBuildingButton(GameState& game_state) :
	BuildingButton(
		TextureID::SpikesIcon,
		game_state,
		make_creator<Spikes>(),
		TileRestrictions::RoadOnly,
		ParamsManager::Instance().params.guns.spikes.cost,
		0,
        BuildingType::Spikes,
        "Шипы"
){}

void SpikesBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
	bool allowed = is_cell_allowed(x_id, y_id);
	sf::Sprite spikes = Spikes::get_sprite_for_tile(x_id, y_id);
	spikes.setOrigin(16, 16);
	spikes.setPosition(x_id * 32 + 16, y_id * 32 + 16);
	if (!allowed) spikes.setColor(sf::Color(255, 0, 0));
	window.draw(spikes);
}


HedgeBuildingButton::HedgeBuildingButton(GameState& game_state):
    BuildingButton(
        TextureID::Hedgehog,
        game_state,
        make_creator<Hedgehog>(),
        TileRestrictions::RoadOnly,
        ParamsManager::Instance().params.guns.spikes.cost,
        0,
        BuildingType::Hedgehogs,
        "Противотанковые ежи"
    )
{}

void HedgeBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
    bool allowed = is_cell_allowed(x_id, y_id);
    sf::Sprite headge(TextureManager::Instance().textures[TextureID::Hedgehog]);
    headge.setOrigin(16, 16);
    headge.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    if (!allowed) headge.setColor(sf::Color(255, 0, 0));
    window.draw(headge);
}


RadarBuildingButton::RadarBuildingButton(GameState& game_state):
    BuildingButton(
        TextureID::RadarIcon,
        game_state,
        [](int x_id, int y_id) {
            auto radar = std::make_unique<Radar>(x_id, y_id);
            NetManager::Instance().new_radar(x_id, y_id, radar.get());
            return radar;
        },
        TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.radar.cost,
        ParamsManager::Instance().params.guns.radar.radius_upgrades[0].radius,
        BuildingType::Radar,
        "Радар"
    )
{}

void RadarBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
    bool allowed = is_cell_allowed(x_id, y_id);
    if (allowed)
        draw_radius(window, x_id, y_id);
    sf::Sprite base(TextureManager::Instance().textures[TextureID::GunBase]);
    base.setPosition(x_id * 32, y_id * 32);
    if (!allowed) base.setColor(sf::Color(255, 0, 0));
    window.draw(base);

    sf::Sprite radar(TextureManager::Instance().textures[TextureID::Radar]);
    radar.setOrigin(16, 16);
    radar.rotate(180);
    radar.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    if (!allowed) radar.setColor(sf::Color(255, 0, 0));
    window.draw(radar);
}


RadioTowerBuildingButton::RadioTowerBuildingButton(GameState& game_state):
    BuildingButton(
        TextureID::RadioTower,
        game_state,
        [](int x_id, int y_id) {
            auto radio_tower = std::make_unique<RadioTower>(x_id, y_id);
            NetManager::Instance().new_radio_tower(x_id, y_id);
            return radio_tower;
        },
        TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.radio_tower.cost,
        ParamsManager::Instance().params.guns.radio_tower.radius,
        BuildingType::RadioTower,
        "Радиовышка"
    )
{}

void RadioTowerBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
    bool allowed = is_cell_allowed(x_id, y_id);
    if (allowed)
        draw_radius(window, x_id, y_id);
    sf::Sprite radio_tower(TextureManager::Instance().textures[TextureID::RadioTower]);
    radio_tower.setPosition(x_id * 32, y_id * 32);
    if (!allowed) radio_tower.setColor(sf::Color(255, 0, 0));
    window.draw(radio_tower);
}
