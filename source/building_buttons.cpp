#include "building_buttons.h"
#include "game_state.h"

#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "guns/minigun.h"
#include "guns/mine.h"
#include "guns/spikes.h"
#include "guns/hedgehog.h"
#include "shader_manager.h"
#include "achievement_system.h"

BuildingButton::BuildingButton(TileTexture gun_icon, GameState& game_state, const BuildingCreator& creator, TileRestrictions restrictions, int cost, float radius, BuildingType type)
    : creator{ creator }, restrictions{ restrictions }, m_gun_icon{ gun_icon }, m_game_state{ game_state }, cost{ cost }, m_radius(radius), m_type{ type } {

    group = tgui::Group::create();
    group->setSize({ "height" , "100%" });

    button = tgui::BitmapButton::create();
	auto button_renderer = button->getRenderer();
	button_renderer->setBorders(0);
	button->setImage(TileMap::Instance().textures[gun_icon]);
	button->setImageScaling(1);
	button->setSize({ "100%" , "100%" });

    group->add(button);

    lock = tgui::Picture::create(TileMap::Instance().textures[TileTexture::Locked]);
    lock->setSize({ "100%" , "100%" });
    lock->ignoreMouseEvents(true);
    group->add(lock);


    m_tooltip = tgui::Panel::create();
    m_tooltip->setTextSize(30);
    m_tooltip->setVisible(false);
    auto tooltip_renderer = m_tooltip->getRenderer();
    tooltip_renderer->setBackgroundColor(tgui::Color::Color(50, 50, 50, 200));
    tooltip_renderer->setBorders(3);
    tooltip_renderer->setBorderColor(tgui::Color::Black);
    tooltip_renderer->setFont(game_state.GOSTtypeA_font);
    m_tooltip->setPosition("100%", "HealthCountWidget.height");
    m_tooltip->setOrigin(1., 0);
    m_tooltip->setSize("35%", "80%");

    m_game_state.get_tgui().add(m_tooltip);


	connect();
    set_state(m_state);
}

void BuildingButton::connect() {
	button->onPress.disconnectAll();
	button->onPress.connect([&]() {
        m_tooltip->setVisible(true); // в любом случае (пока)
        for (auto& button : m_game_state.m_building_buttons) {
            if (button.get() != this)
                button->unselect();
        }
        if (m_state == State::Active) {
            set_state(State::Selected);
            m_game_state.m_current_building_construction = this;
        }
	});
}

BuildingButton::BuildingButton(BuildingButton&& btn) :
    creator{ btn.creator }, restrictions{ btn.restrictions }, m_gun_icon{ btn.m_gun_icon }, m_game_state{ btn.m_game_state }, cost{ btn.cost }, m_radius{ btn.m_radius }, m_type{btn.m_type}
{
	button = std::move(btn.button);
	connect(); // нужно переназнывать, поскольку используем this в  lambda-функции.
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
	auto& gun_texture = TileMap::Instance().textures[m_gun_icon];
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

void BuildingButton::set_grayscale() {
    tgui::Texture background(TileMap::Instance().textures[TileTexture::ButtonBackground]);
    tgui::Texture icon(TileMap::Instance().textures[m_gun_icon]);
    background.setShader(&ShaderManager::Instance().shaders[Shader::GrayScale]);
    icon.setShader(&ShaderManager::Instance().shaders[Shader::GrayScale]);
    button->getSharedRenderer()->setTexture(background);
    button->setImage(icon);
}

void BuildingButton::set_state(State state) {
    switch (state) {
    case BuildingButton::State::Locked: {
        set_grayscale();
        lock->setVisible(true);
        break;
    }
    case BuildingButton::State::Active: {
        lock->setVisible(false);
        tgui::Texture background(TileMap::Instance().textures[TileTexture::ButtonBackground]);
        tgui::Texture icon(TileMap::Instance().textures[m_gun_icon]);
        button->getSharedRenderer()->setTexture(background);
        button->setImage(icon);
        break;
    }
    case BuildingButton::State::Selected: {
        lock->setVisible(false);
        tgui::Texture background(TileMap::Instance().textures[TileTexture::ButtonClickedBackground]);
        tgui::Texture icon(TileMap::Instance().textures[m_gun_icon]);
        button->getSharedRenderer()->setTexture(background);
        button->setImage(icon);
        break;
    }
    case BuildingButton::State::Disabled:
        set_grayscale();
        break;
    default:
        break;
    }
    m_state = state;
}

void BuildingButton::unselect() {
    m_tooltip->setVisible(false); // в любом случае
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

std::string to_string_2(double number) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << number;
    return out.str();
}

MinigunBuildingButton::MinigunBuildingButton(GameState& game_state):
	BuildingButton(
		TileTexture::MinigunIcon,
		game_state,
		make_creator<MiniGun>(),
		TileRestrictions::NoRoads,
		ParamsManager::Instance().params.guns.minigun.cost,
		ParamsManager::Instance().params.guns.minigun.radius,
        BuildingType::Minigun
	)
{
    auto& params = ParamsManager::Instance().params.guns.minigun;
    auto label = tgui::RichTextLabel::create();
    label->getRenderer()->setTextColor(tgui::Color::White);
    std::string description =
        "<color=#ffd303>Пулемёт</color>\n"
        "<b>Описание:</b> Первоначальное, доволько капризное орудие. При стрельбе пулемет нагревается. "
        "Существует критическая температура. Время работы пулемета при нагреве выше критичесокой температуры ограничено. "
        "При превышении ограничения наступает перегрев: пулемет перестает стрелять до тех пор, пока не охладиться. "
        "Урон бронепробиваемость и скорострельность пулемета растут с повышением температуры.\n"
        "<color=#ffd303>Стоимость:" + std::to_string(params.cost) + "</color>\n"
        "<b>Характеристики:</b>\n"
        "радиус поражения: " + to_string_2(params.radius) + "\n"
        "урон при наименьшем нагреве: " + std::to_string(params.min_damage) + "\n"
        "урон при максимальном нагреве: " + std::to_string(params.max_damage) + "\n"
        "бронепробиваемость при наименьшем нагреве: " + std::to_string(params.penetration_upgrades[0].min_armor_penetration_level) + "\n"
        "бронепробиваемость при наибольшем нагреве: " + std::to_string(params.penetration_upgrades[0].max_armor_penetration_level) + "\n"
        "частота выстрелов в секунду при минимальном нагреве:" + to_string_2(params.min_rotation_speed / 60.) + "\n"
        "частота выстрелов в секунду при максимальном нагреве:" + to_string_2(params.max_rotation_speed / 60.) + "\n"
        "время до максимального нагрева: " + to_string_2(params.heating_time) + "\n"
        "время охлажения с максимлаьного нагрева до холодного состояния: " + to_string_2(params.cooling_time) + "\n"
        "минимальная температура: 0\n"
        "максимальная температура: 1000\n"
        "значение критической температуры: " + to_string_2(params.critical_temperature * 1000) + "\n"
        "время работы при критической температуре (до перегрева): " + to_string_2(params.critical_temperature_work_duration) + "\n"
        "время воосстановления после перегрева: " + to_string_2(params.cooldown_duration) + "\n";
    label->setText(description);
    label->setSize("100%", "100%");
    m_tooltip->add(label);

    lock_button(false);

}

MineBuildingButton::MineBuildingButton(GameState& game_state):
	BuildingButton(
		TileTexture::Mine,
		game_state,
		make_creator<Mine>(),
		TileRestrictions::RoadOnly,
		ParamsManager::Instance().params.guns.mine.cost,
		ParamsManager::Instance().params.guns.mine.damage_radius,
        BuildingType::Mine
	) 
{
    auto& params = ParamsManager::Instance().params.guns.mine;
    auto label = tgui::RichTextLabel::create();
    label->getRenderer()->setTextColor(tgui::Color::White);
    std::string description =
        "<color=#ffd303>Мина</color>\n"
        "<b>Описание:</b> при активации взрывается, нанося всем противникам урон в радиусе поражения. "
        "Чем дальше противник от эпицентра взрыва, тем меньший урон он получит.\n"
        "<color=#ffd303>Стоимость:" + std::to_string(params.cost) + "</color>\n"
        "<b>Характеристики:</b>\n"
        "радиус поражения: " + to_string_2(params.damage_radius) + "\n"
        "урон в эпицентре: " + std::to_string(params.max_damage) + "\n"
        "урон на радиусе: " + std::to_string(params.min_damage) + "\n";
        "бронепробиваемость" + std::to_string(params.armor_penetration_level) + "\n";
    label->setText(description);
    label->setSize("100%", "100%");
    m_tooltip->add(label);

}

TwinGunBuildingButton::TwinGunBuildingButton(GameState& game_state) :
	BuildingButton(
		TileTexture::TwingunIcon,
		game_state,
		make_creator<TwinGun>(),
		TileRestrictions::NoRoads,
		ParamsManager::Instance().params.guns.twingun.cost,
		ParamsManager::Instance().params.guns.twingun.radius,
        BuildingType::TwinGun
	)
{}

void TwinGunBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
	bool allowed = is_cell_allowed(x_id, y_id);
	if (allowed) {
		draw_radius(window, x_id, y_id);
	}
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

AntitankGunBuildingButton::AntitankGunBuildingButton(GameState& game_state) :
	BuildingButton(
		TileTexture::AntitankGunIcon,
		game_state,
		make_creator<AntitankGun>(),
		TileRestrictions::NoRoads,
		ParamsManager::Instance().params.guns.antitank.cost,
		ParamsManager::Instance().params.guns.antitank.radius,
        BuildingType::AntitankGun
	)
{
    auto& params = ParamsManager::Instance().params.guns.antitank;
    auto label = tgui::RichTextLabel::create();
    label->getRenderer()->setTextColor(tgui::Color::White);
    std::string description =
        "<color=#ffd303>Противотанковая пушка</color>\n"
        "<b>Описание:</b> Мощное орудие, имеющее высокий уровень бронепробиваемости и широкий радиус поражения. \n"
        "<color=#ffd303>Стоимость:" + std::to_string(params.cost) + "</color>\n"
        "<b>Характеристики:</b>\n"
        "радиус поражения: " + to_string_2(params.radius) + "\n"
        "урон: " + std::to_string(params.damage) + "\n"
        "время преезарядки: " + to_string_2(params.cooldown) + "\n"
        "бронепробиваемость: " + std::to_string(params.armor_penetration_level) + "\n";
    label->setText(description);
    label->setSize("100%", "100%");
    m_tooltip->add(label);

}

void AntitankGunBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
	bool allowed = is_cell_allowed(x_id, y_id);
	if (allowed) {
		draw_radius(window, x_id, y_id);
	}
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

SpikesBuildingButton::SpikesBuildingButton(GameState& game_state) :
	BuildingButton(
		TileTexture::SpikesIcon,
		game_state,
		make_creator<Spikes>(),
		TileRestrictions::RoadOnly,
		ParamsManager::Instance().params.guns.spikes.cost,
		0,
        BuildingType::Spikes
	) {
    auto& params = ParamsManager::Instance().params.guns.spikes;
    auto label = tgui::RichTextLabel::create();
    label->getRenderer()->setTextColor(tgui::Color::White);
    std::string description =
        "<color=#ffd303>Шипы</color>\n"
        "<b>Описание:</b> Прокалывает колеса, в результате чего противник останавливается не некоторое время. "
        "Шипы теряют прочность каждый раз, когда останавливают противника. При достижении нулевой прочности шипы ломаются. "
        "Техника с тяжелыми гусеницами моментально уничтожает шипы. "
        "Бесполезны против пехоты и гусенечной техники. \n"
        "<color=#ffd303>Стоимость:" + std::to_string(params.cost) + "</color>\n"
        "<b>Характеристики:</b>\n"
        "длительность остановки противника: " + to_string_2(params.delay) + "\n"
        "прочность: " + std::to_string(params.health) + "\n";
    label->setText(description);
    label->setSize("100%", "100%");
    m_tooltip->add(label);
}
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
        TileTexture::Hedgehog,
        game_state,
        make_creator<Hedgehog>(),
        TileRestrictions::RoadOnly,
        ParamsManager::Instance().params.guns.spikes.cost,
        0,
        BuildingType::Hedgehogs
    )
{
    auto& params = ParamsManager::Instance().params.guns.hedgehog;
    auto label = tgui::RichTextLabel::create();
    label->getRenderer()->setTextColor(tgui::Color::White);
    std::string description =
        "<color=#ffd303>Противотанковые ежи</color>\n"
        "<b>Описание:</b> Останавливают крупную колесную и гусенечную технику на некоторое время."
        "Теряют прочность каждый раз, когда останавливают гусенечную технику. При достижении нулевой прочности ломаются."
        "Колесная техника урон не наносит. "
        "Техника с тяжелыми гусеницами моментально уничтожает противотанковые ежи. "
        "Колесная техника останавливается на большее время. Бесполезны против пехоты и мотоциклистов.\n"
        "<color=#ffd303>Стоимость:" + std::to_string(params.cost) + "</color>\n"
        "<b>Характеристики:</b>\n"
        "длительность остановки гусенечной техники: " + to_string_2(params.delay) + "\n"
        "длительность остановки колесной техники: " + to_string_2(params.delay * params.wheels_debuff) + "\n"
        "прочность: " + std::to_string(params.health) + "\n";
    label->setText(description);
    label->setSize("100%", "100%");

    m_tooltip->add(label);
}

void HedgeBuildingButton::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
    bool allowed = is_cell_allowed(x_id, y_id);
    sf::Sprite headge(TileMap::Instance().textures[TileTexture::Hedgehog]);
    headge.setOrigin(16, 16);
    headge.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    if (!allowed) headge.setColor(sf::Color(255, 0, 0));
    window.draw(headge);
}
