#include "building_buttons.h"
#include "game_state.h"

#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "shader_manager.h"

BuildingButton::BuildingButton(TileTexture gun_icon, GameState& game_state, const BuildingCreator& creator, TileRestrictions restrictions, int cost)
	: creator{ creator }, restrictions{ restrictions }, m_gun_icon{ gun_icon }, m_game_state{ game_state }, cost{cost} {
	button = tgui::BitmapButton::create();
	auto button_renderer = button->getRenderer();
	button_renderer->setTexture(TileMap::Instance().textures[TileTexture::ButtonBackground]);
	button_renderer->setBorders(0);
	button->setImage(TileMap::Instance().textures[gun_icon]);
	button->setImageScaling(1);
	button->setSize({ "height" , "100%" });
	connect();
}

void BuildingButton::connect() {
	button->onPress.disconnectAll();
	button->onPress.connect([&]() {
		if (disabled) return;
		m_game_state.m_current_building_construction = this;
		this->button->getRenderer()->setTexture(TileMap::Instance().textures[TileTexture::ButtonClickedBackground]);
		for (auto& button : m_game_state.m_building_buttons) {
			if (button.get() != this)
				button->disable_selection();
		}
	});
}

BuildingButton::BuildingButton(BuildingButton&& btn) :
	creator{ btn.creator }, restrictions{ btn.restrictions }, m_gun_icon{ btn.m_gun_icon }, m_game_state{ btn.m_game_state }, cost{ btn.cost }
{
	button = std::move(btn.button);
	connect(); // нужно переназнывать, поскольку используем this в  lambda-функции.
}

void BuildingButton::coins_update(int current_coins_count) {
	bool d = current_coins_count < cost;
	if (disabled != d) {
		disabled = d;
		tgui::Texture background(TileMap::Instance().textures[TileTexture::ButtonBackground]);
		tgui::Texture icon(TileMap::Instance().textures[m_gun_icon]);
		if (disabled) {
			background.setShader(&ShaderManager::Instance().shaders[Shader::GrayScale]);
			icon.setShader(&ShaderManager::Instance().shaders[Shader::GrayScale]);
			button->getSharedRenderer()->setTexture(background);
			button->setImage(icon);
		}
		else {
			button->getSharedRenderer()->setTexture(background);
			button->setImage(icon);
		}
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
	auto& gun_texture = TileMap::Instance().textures[m_gun_icon];
	sf::Sprite gun(gun_texture);
	gun.setOrigin(gun_texture.getSize().x / 2.f, gun_texture.getSize().y / 2.f);
	gun.rotate(180);
	gun.setPosition(x_id * 32 + 16, y_id * 32 + 16);
	if (!allowed) gun.setColor(sf::Color(255, 0, 0));
	window.draw(gun);
}

void BuildingButton::disable_selection() {
	if (!disabled)
		button->getRenderer()->setTexture(TileMap::Instance().textures[TileTexture::ButtonBackground]);
}


TwinGunBuildingButton::TwinGunBuildingButton(GameState& game_state, int cost) :
	BuildingButton(TileTexture::TwingunIcon, game_state, make_creator<TwinGun>(), TileRestrictions::NoRoads, cost)
{}

void TwinGunBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
	bool allowed = is_cell_allowed(x_id, y_id);
	sf::Sprite base(TileMap::Instance().textures[TileTexture::GunBase]);
	base.setPosition(x_id * 32, y_id * 32);
	if (!allowed) base.setColor(sf::Color(255, 0, 0));
	window.draw(base);

	sf::Sprite gun(TileMap::Instance().textures[TileTexture::TwingunConstructed]);
	gun.setOrigin(16, 15.5);
	gun.rotate(180);
	gun.setPosition(x_id * 32 + 16, y_id * 32 + 16);
	if (!allowed) gun.setColor(sf::Color(255, 0, 0));
	window.draw(gun);
}

AntitankGunBuildingButton::AntitankGunBuildingButton(GameState& game_state, int cost) :
	BuildingButton(TileTexture::AntitankGunIcon, game_state, make_creator<AntitankGun>(), TileRestrictions::NoRoads, cost)
{}

void AntitankGunBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
	bool allowed = is_cell_allowed(x_id, y_id);
	sf::Sprite base(TileMap::Instance().textures[TileTexture::GunBase]);
	base.setPosition(x_id * 32, y_id * 32);
	if (!allowed) base.setColor(sf::Color(255, 0, 0));
	window.draw(base);

	sf::Sprite gun(TileMap::Instance().textures[TileTexture::AntitankGunConstructed]);
	gun.setOrigin(9, 16);
	gun.rotate(180);
	gun.setPosition(x_id * 32 + 16, y_id * 32 + 16);
	if (!allowed) gun.setColor(sf::Color(255, 0, 0));
	window.draw(gun);
}