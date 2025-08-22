#include "game_state.h"

GameState::GameState(sf::RenderWindow& window): gui(window) {
	gui.setFont(tgui::Font("PixelSplitter-Bold.ttf"));


	player_health_count_widget = tgui::Label::create("X" + std::to_string(player_hp));
	player_health_count_widget->setPosition("100%", 0);
	player_health_count_widget->setOrigin(1, 0);
	player_health_count_widget->setTextSize(32);
	player_health_count_widget->ignoreMouseEvents(true);
	player_health_count_widget->getRenderer()->setTextColor(tgui::Color::White);
	gui.add(player_health_count_widget, "HealthCountWidget");


	tgui::Picture::Ptr heart_icon = tgui::Picture::create("sprites/heart.png");
	heart_icon->setPosition(
		"(HealthCountWidget.left - width)", // X: левый край rightWidget минус ширина heart_icon
		"(HealthCountWidget.top + 5)"       // Y: на той же высоте
	);
	heart_icon->setSize(32, 32);

	gui.add(heart_icon);

	centered_message = tgui::Label::create("");
	centered_message->setPosition("50%", "50%");
	centered_message->setOrigin(0.5, 0.5);
	centered_message->setTextSize(128);
	centered_message->ignoreMouseEvents(true);
	centered_message->getRenderer()->setTextColor(tgui::Color::Red);

	gui.add(centered_message);


	minigun_state = tgui::Label::create();
	minigun_state->setPosition("0%", "0%");
	minigun_state->setTextSize(16);
	minigun_state->ignoreMouseEvents(true);
	minigun_state->getRenderer()->setTextColor(tgui::Color::White);
	gui.add(minigun_state);
}

tgui::Gui& GameState::get_tgui() {
	return gui;
}

void GameState::logic() {
	if (is_game_over()) {
		centered_message->setText("GAME OVER");
	}
}

void GameState::player_health_add(int health) {
	player_hp += health;
	player_health_count_widget->setText("X" + std::to_string(player_hp));
}

void GameState::minigun_state_update(const MiniGun& minigun) {
	std::string message;
	std::unordered_map<MiniGun::State, std::string> state_to_string{
		{MiniGun::State::CoolDown, "CoolDown"},
		{MiniGun::State::Cooling, "Cooling"},
		{MiniGun::State::Heating, "Heating"},
		{MiniGun::State::Ready, "Ready"}
	};
	std::unordered_map<MiniGun::ShootState, std::string> shoot_state_to_string{
		{MiniGun::ShootState::CoolDown, "CoolDown"},
		{MiniGun::ShootState::Ready, "Ready"}
	};
	message += "State: " + state_to_string[minigun.m_state] + "\n";
	message += "Temperature: " + std::to_string(minigun.m_temperature / (1000 * 1000)) + "\n";
	message += "m_cooldown_timer: " + std::to_string(minigun.m_cooldown_timer / (1000 * 1000)) + "\n";
	message += "m_critical_temperature_mod_timer: " + std::to_string(minigun.m_critical_temperature_mod_timer / (1000 * 1000)) + "\n";
	message += "m_shoot_timer: " + std::to_string(minigun.m_shoot_timer / (1000 * 1000)) + "\n";
	message += "Shoot State: " + shoot_state_to_string[minigun.m_shoot_state] + "\n";

	minigun_state->setText(message);

}