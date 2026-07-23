#include "guns/hedgehog.h"
#include "enemy_manager.h"
#include "texture_manager.h"
#include "sound_manager.h"
#include "game_state.h"

Hedgehog::Hedgehog(int x_id, int y_id): BuildingWithHealth(x_id, y_id, BuildingType::Hedgehogs), params(ParamsManager::Instance().params.guns.hedgehog) {
    set_health(params.health);
}

void Hedgehog::draw(sf::RenderWindow& window) {
    sf::Sprite sprite(TextureManager::Instance().textures[TextureID::Hedgehog]);
    sprite.setPosition(x_id * 32, y_id * 32);
    window.draw(sprite);
}

void Hedgehog::logic(double dtime) {
    glm::vec2 pos(x_id * 32 + 16, y_id * 32 + 16);
    int health = get_health();
    for (auto enemy : EnemyManager::Instance().get_enemy_container()) {
        if (enemy->infantry || enemy->wheels == IEnemy::Wheels::None)
            continue;
        if (health <= 0)
            return;
        float debuff = enemy->wheels == IEnemy::Wheels::Wheels ? params.wheels_debuff : 1.f;
        if (glm::length(pos - enemy->get_position()) < 0.2 * 32) {
            if (enemy->wheels == IEnemy::Wheels::HeavyTracks)
                health = 0;
            else if (enemy->break_enemy(params.delay * debuff) && enemy->wheels == IEnemy::Wheels::Tracks)
                --health;
            if (health <= 0) {
                if (auto_repair && enemy->wheels != IEnemy::Wheels::HeavyTracks && GameState::Instance().get_player_coins() >= params.cost) {
                    health = params.health;
                    GameState::Instance().player_coins_add(-params.cost);
                }
                else {
                    SoundManager::Instance().play(Sounds::HedgehogsBreaking);
                    set_health(health);
                    return;
                }
            }
        }
    }
    set_health(health);
}
