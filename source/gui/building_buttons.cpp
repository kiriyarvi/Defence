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


BuildingPanel::BuildingPanel(Widget* ui) : ui{ui} {
    m_parent = ui;

    Widget* b1 = add(std::make_unique<NMinigunBuildingButton>());
    Widget* b2 = add(std::make_unique<NMineBuildingButton>());
    Widget* b3 = add(std::make_unique<NSpikesBuildingButton>());
    Widget* b4 = add(std::make_unique<NHedgehogBuildingButton>());
    Widget* b5 = add(std::make_unique<NAntitankBuildingButton>());
    Widget* b6 = add(std::make_unique<NTwinGunBuildingButton>());
    Widget* b7 = add(std::make_unique<NRadarBuildingButton>());
    Widget* b8 = add(std::make_unique<NRadioMastBuildingButton>());

    auto prev_it = m_children.end();
    for (auto button_it = m_children.begin(); button_it != m_children.end(); ++button_it) {
        Widget* button = button_it->get();
        button->add_rule(Property::SIZE, [ui](LayoutNode::Layout& layout) {
            layout.height = ui->layout.height * 0.1;
            layout.width = layout.height;
        }, { { ui, Property::HEIGHT } });
        Widget* prev = nullptr;
        if (prev_it != m_children.end())
            prev = prev_it->get();
        Widget* button_panel = this;
        button->add_rule(Property::POSITION, [button_panel, prev](LayoutNode::Layout& layout) {
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
        }, { {button_panel, Property::WIDTH} });
        prev_it = button_it;
    }

    //position_anchor(Anchor::LEFT | Anchor::T, ui, Anchor::LEFT);
    add_rule(Property::WIDTH, [ui](LayoutNode::Layout& layout) {
        layout.width = ui->layout.width * 0.7;
    }, { {ui, Property::WIDTH} });
    Widget* last_button = m_children.back().get();
    add_rule(Property::HEIGHT, [last_button](LayoutNode::Layout& layout) {
        layout.height = last_button->layout.y + last_button->layout.height;
    }, { {last_button, Property::Y | Property::HEIGHT} });

}

void BuildingPanel::update(int player_coins) {
    for (auto& button : m_children)
        dynamic_cast<NBuildingButton*>(button.get())->update(player_coins);
    if (m_selected_button && m_selected_button->m_state != NBuildingButton::State::SELECTED)
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

void BuildingPanel::select(NBuildingButton* button_to_select) {
    for (auto& button_ptr : m_children) {
        NBuildingButton* button = dynamic_cast<NBuildingButton*>(button_ptr.get());
        if (button == button_to_select) {
            button->set_state(NBuildingButton::State::SELECTED);
            m_selected_button = button_to_select;
        }
        else if (button->m_state == NBuildingButton::State::SELECTED)
            button->set_state(NBuildingButton::State::ACTIVE);
    }
}

void BuildingPanel::unselect() {
    if (m_selected_button) {
        assert(m_selected_button->m_state == NBuildingButton::State::SELECTED);
        m_selected_button->set_state(NBuildingButton::State::ACTIVE);
    }
    m_selected_button = nullptr;
}

void BuildingPanel::unselect(NBuildingButton* button) {
    if (m_selected_button == button) {
        m_selected_button->set_state(NBuildingButton::State::ACTIVE);
        m_selected_button = nullptr;
    }
}

void BuildingPanel::draw_building_plan(sf::RenderWindow& window, int x_id, int y_id) {
    if (!m_selected_button)
        return;
    m_selected_button->draw_building_plan(window, x_id, y_id);
}

NBuildingButton::NBuildingButton(const BuildingCreator& creator, BuildingType type, TileRestrictions restrictions, int cost, float radius, TextureID icon):
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
        Panel* panel = (Panel*)building_panel->ui->add(Panel::create(sf::Color(50, 50, 50, 255), sf::Color::Black, 0));
        Label* label = (Label*)panel->add(Label::create(true));
        label->add_text(to_string(m_type) + "\n", sf::Color::White, sf::Text::Style::Bold);
        label->add_text("Стоимость: " + std::to_string(m_cost) + "\n", Label::gold_color);
        if (m_state == State::UNDISCOVERED)
            label->add_text("Закрыто: " + AchievementSystem::Instance().get_building_unlock_condition_description(m_type) + "\n", sf::Color::Red);
        label->add_text("Откройте справку, для получения подробностей", sf::Color::White, sf::Text::Style::Italic);
        
        panel->size_include(label);
        panel->position_tooltip(building_panel->ui, Anchor::BOTTOM | Anchor::LEFT);
        panel->receive_mouse_events = false;
        m_tooltip = panel;
    };

    on_unhovered = [this](const glm::vec2& position_transform, const glm::vec2& mouse_pos) {
       BuildingPanel* building_panel = dynamic_cast<BuildingPanel*>(m_parent);
       building_panel->ui->delete_widget(m_tooltip);
    };
}

void NBuildingButton::update(int current_coins_count) {
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


void NBuildingButton::set_state(State state) {
    if (m_state == state)
        return;
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
    m_state = state;
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

void NBuildingButton::draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) {
    sf::Texture& texture = TextureManager::Instance().textures[m_icon];
    sf::Sprite building(texture);
    building.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    building.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
    building.rotate(180);
    if (!allowed) building.setColor(sf::Color(255, 0, 0));
    window.draw(building);
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

NMinigunBuildingButton::NMinigunBuildingButton():
    NBuildingButton(
        NBuildingButton::make_creator<MiniGun>(),
        BuildingType::Minigun,
        NBuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.minigun.cost,
        ParamsManager::Instance().params.guns.minigun.radius,
        TextureID::MinigunIcon
        )
{}

NMineBuildingButton::NMineBuildingButton() :
    NBuildingButton(
        NBuildingButton::make_creator<Mine>(),
        BuildingType::Mine,
        NBuildingButton::TileRestrictions::RoadOnly,
        ParamsManager::Instance().params.guns.mine.cost,
        ParamsManager::Instance().params.guns.mine.damage_radius,
        TextureID::Mine
    ) {}


NSpikesBuildingButton::NSpikesBuildingButton():
    NBuildingButton(
        NBuildingButton::make_creator<Spikes>(),
        BuildingType::Spikes,
        NBuildingButton::TileRestrictions::RoadOnly,
        ParamsManager::Instance().params.guns.spikes.cost,
        0,
        TextureID::SpikesIcon
    ) {}

void NSpikesBuildingButton::draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) {
    sf::Sprite spikes = Spikes::get_sprite_for_tile(x_id, y_id);
    spikes.setOrigin(16, 16);
    spikes.setPosition(x_id * 32 + 16, y_id * 32 + 16);
    if (!allowed) spikes.setColor(sf::Color(255, 0, 0));
    window.draw(spikes);
}

NHedgehogBuildingButton::NHedgehogBuildingButton():
    NBuildingButton(
        NBuildingButton::make_creator<Hedgehog>(),
        BuildingType::Hedgehogs,
        NBuildingButton::TileRestrictions::RoadOnly,
        ParamsManager::Instance().params.guns.hedgehog.cost,
        0,
        TextureID::Hedgehog
    ) {}

NAntitankBuildingButton::NAntitankBuildingButton():
    NBuildingButton(
        NBuildingButton::make_creator<AntitankGun>(),
        BuildingType::AntitankGun,
        NBuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.antitank.cost,
        ParamsManager::Instance().params.guns.antitank.radius,
        TextureID::AntitankGunIcon
    ) {}

void NAntitankBuildingButton::draw_building(sf::RenderWindow& window, int x_id, int y_id, bool allowed) {
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


NTwinGunBuildingButton::NTwinGunBuildingButton():
    NBuildingButton(
        NBuildingButton::make_creator<TwinGun>(),
        BuildingType::TwinGun,
        NBuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.twingun.cost,
        ParamsManager::Instance().params.guns.twingun.radius,
        TextureID::TwingunIcon
    ) {}

NRadarBuildingButton::NRadarBuildingButton():
    NBuildingButton(
        [](int x_id, int y_id) {
            auto radar = std::make_unique<Radar>(x_id, y_id);
            NetManager::Instance().new_radar(x_id, y_id, radar.get());
            return radar;
        },
        BuildingType::Radar,
        NBuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.radar.cost,
        ParamsManager::Instance().params.guns.radar.radius_upgrades[0].radius,
        TextureID::RadarIcon
    ) {}

NRadioMastBuildingButton::NRadioMastBuildingButton():
    NBuildingButton(
        [](int x_id, int y_id) {
            auto radio_tower = std::make_unique<RadioTower>(x_id, y_id);
            NetManager::Instance().new_radio_tower(x_id, y_id);
            return radio_tower;
        },
        BuildingType::RadioTower,
        NBuildingButton::TileRestrictions::NoRoads,
        ParamsManager::Instance().params.guns.radio_tower.cost,
        ParamsManager::Instance().params.guns.radio_tower.radius,
        TextureID::RadioTower
    ) {}


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
