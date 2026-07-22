#include "game_state.h"

#include "guns/mine.h"
#include "guns/minigun.h"
#include "guns/twin_gun.h"
#include "guns/antitank_gun.h"
#include "achievement_system.h"
#include "enemy_manager.h"
#include "resource_manager.h"
#include "sound_manager.h"
#include "animation_holder.h"
#include "debugger.h"

#include "gui/label.h"
#include "gui/icon.h"
#include "gui/scale.h"


GameState::GameState(sf::RenderWindow& window): m_ui{*this, window }
{
    m_ui.create();          //создаем интерфейс.
    m_map.generate_map();   //сгенерируем карту
    m_ui.get_console()->add_message("Нажмите R чтобы перегенерировать карту и Q, чтобы подтвердить выбор.");
}

void GameState::set_game_finished_state(bool win) {
    m_win = win;
    m_ui.game_over(win);
    m_state = State::GAME_FINISHED;
}

GameState::~GameState() {

}

void GameState::on_event(sf::Event& event) {
    m_ui.on_event(event);
}

void GameState::logic(double dtime_mc) {
    float time_multiplier = GameState::Instance().get_time_multiplier();
    EnemyManager::Instance().logic(time_multiplier * dtime_mc);     //1. Логика врагов: уничтожение, перемещение по маршруту, волны, анимации дымовых завес
    m_map.logic(time_multiplier * dtime_mc);                        //2. Логика построек.
    NetManager::Instance().logic(time_multiplier * dtime_mc);       //3. Радары, объединенные в сеть не вычисляют свою логику. Логика единая на сеть - вычислим её.
    SoundManager::Instance().logic();                               //4. Проигрывание звуков
    AnimationHolder::Instance().logic(time_multiplier * dtime_mc);  //5. Отложенные анимации
    m_ui.logic(dtime_mc);                                           //6. Анимации пользовательского интерфейса.
}

void GameState::draw(sf::RenderWindow& window) {
    m_ui.get_camera().apply(window);
    window.clear(sf::Color::Black);
    m_map.draw(window);                             //1. TileMap
    m_ui.draw_on_map_effects();                     //2. Обозначение маршрутов вражеских войск.
    EnemyManager::Instance().draw(window);          //3. Вражеские войска
    m_map.draw_effects(window);                     //4. Анимации взрывов и выстрелов от зданий
    AnimationHolder::Instance().draw(window);       //5. Отложенные анимации. Используется только в CRUISER. TODO: уточнить, можно ли без этого обойтись
    EnemyManager::Instance().draw_effects(window);  //6. Анимации уничтожения врагов, дым, обозначения рассекреченных врагов
    Debugger::Instance().draw(window);              //7. Всякие дебажные штуки поверх
    GUI::Instance().draw(window);                   //8. Интерфейс
}

void GameState::enemy_defeated(EnemyType type) {
    bool achievement = AchievementSystem::Instance().defeated(type);
    if (!achievement)
        return;
    if (m_state != State::GAME && m_state != State::PREPAIRING)
        return;
    m_ui.update_on_enemy_defeated(m_player_coins);
}

void GameState::win() {
    set_game_finished_state(true);
}

void GameState::init_stage(int stage) {
    if (stage == 0) {
        player_coins_add(1000);
    } else {
        AchievementSystem::Instance().unlock_all();
        player_coins_add(10000);
    }
}

void GameState::player_health_add(int health) {
	m_player_hp += health;
    m_ui.update_player_health(m_player_hp);
    if (m_player_hp <= 0)
        set_game_finished_state(false);
}

void GameState::kill_player() {
    player_health_add(-m_player_hp);
}

void GameState::player_coins_add(int coins) {
    if (m_state != State::GAME && m_state != State::PREPAIRING)
        return;
	m_player_coins += coins;
    m_ui.update_player_coins(m_player_coins);
}

void GameState::wave(bool started) {
    m_ui.update_wave_info(started);
}


void GameState::start_game() {
    m_state = State::GAME;
    EnemyManager::Instance().generate_waves();
    init_stage(1);
}
