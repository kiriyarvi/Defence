#include "gui/building_buttons.h"
#include "game_state.h"

#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "guns/minigun.h"
#include "guns/mine.h"
#include "guns/spikes.h"
#include "guns/hedgehog.h"
#include "shader_manager.h"
#include "achievement_system.h"
#include "gui/info_panel.h"

BuildingButton::BuildingButton(TextureID gun_icon, GameState& game_state, const BuildingCreator& creator, TileRestrictions restrictions, int cost, float radius, BuildingType type, const std::string& name)
    : creator{ creator }, restrictions{ restrictions }, m_game_state{ game_state }, cost{ cost }, m_radius(radius), m_type{ type }, m_name{name}, IconButton(gun_icon, TextureID::ButtonBackground, TextureID::ButtonClickedBackground) {
    connect();
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
{
    lock_button(false);
}


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
