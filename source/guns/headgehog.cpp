#include "guns/hedgehog.h"
#include "tile_map.h"
#include "enemy_manager.h"

Hedgehog::Hedgehog(): params(ParamsManager::Instance().params.guns.hedgehog) {
    health = params.health;
}

void Hedgehog::draw(sf::RenderWindow& window, int x, int y) {
    sf::Sprite sprite(TileMap::Instance().textures[TileTexture::Hedgehog]);
    sprite.setPosition(x * 32, y * 32);
    window.draw(sprite);
}

void Hedgehog::logic(double dtime, int x_id, int y_id) {
    glm::vec2 pos(x_id * 32 + 16, y_id * 32 + 16);
    for (auto& enemy : EnemyManager::Instance().m_enemies) {
        if (enemy->wheels == IEnemy::Wheels::HeavyTracks) {
            health = 0;
            return;
        }
        if (enemy->infantry || enemy->wheels != IEnemy::Wheels::Tracks && enemy->wheels != IEnemy::Wheels::Wheels)
            continue;
        if (health <= 0)
            return;
        float debuff = enemy->wheels == IEnemy::Wheels::Wheels ? params.wheels_debuff : 1.f;
        if (glm::length(pos - enemy->get_position()) < 0.2 * 32) {
            if (enemy->break_enemy(params.delay * debuff))
                --health;
        }
    }
}
