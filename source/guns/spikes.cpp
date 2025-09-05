#include "guns/spikes.h"
#include "enemy_manager.h"
#include "texture_manager.h"
#include "sound_manager.h"
#include "game_state.h"

Spikes::Spikes(): params(ParamsManager::Instance().params.guns.spikes) 
{
	set_health(params.health);
}

sf::Sprite Spikes::get_sprite_for_tile(int x_id, int y_id) {
	auto& roads = TileMap::Instance().map[x_id][y_id].roads;
	int tile_id = 0;
	for (int i = 0; i < 4; ++i) {
		tile_id *= 2;
		tile_id += roads[i];
	}
	int count = std::count(roads.begin(), roads.end(), true);
	sf::Sprite sprite;
	sprite.setOrigin(16, 16);
	sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
	float rotation = 0;
	if (count == 4 || count == 0) { // count == 0 для того чтобы отображадся план постройки.
		sprite.setTexture(TextureManager::Instance().textures[TextureID::SpikesCross]);
	}
	else if (count == 2) {
		if (tile_id % 3 == 0) {
			//  turn
			if (tile_id == 3)
				rotation = -90;
			else if (tile_id == 12)
				rotation = 90;
			else if (tile_id == 9)
				rotation = 180;
			sprite.setTexture(TextureManager::Instance().textures[TextureID::SpikesD]);
		}
		else { // прямая дорога
			sprite.setTexture(TextureManager::Instance().textures[TextureID::SpikesRight]);
			if (tile_id == 10) {
				rotation = 90;
			}
		}
	}
	else if (count == 3) {
		sprite.setTexture(TextureManager::Instance().textures[TextureID::SpikesT]);
		if (tile_id == 14)
			rotation = -90;
		else if (tile_id == 11)
			rotation = 90;
		else if (tile_id == 13)
			rotation = 180;

	}
	sprite.setRotation(rotation);
	return sprite;
}

void Spikes::draw(sf::RenderWindow& window, int x_id, int y_id) {
	if (sprite.getTexture() == nullptr)
		sprite = get_sprite_for_tile(x_id, y_id);
	window.draw(sprite);
}


void Spikes::logic(double dtime, int x_id, int y_id) {
    if (get_health() <= 0)
        return;
	glm::vec2 pos(x_id * 32 + 16, y_id * 32 + 16);
    int health = get_health();
	for (auto& enemy : EnemyManager::Instance().m_enemies) {
        if (enemy->wheels == IEnemy::Wheels::None || enemy->wheels == IEnemy::Wheels::Tracks)
            continue;
		if (glm::length(pos - enemy->get_position()) < 0.2 * 32) {
            if (enemy->wheels == IEnemy::Wheels::HeavyTracks)
                health = 0;
            else if (enemy->break_enemy(params.delay))
                --health;
            if (health <= 0) {
                if (auto_repair && enemy->wheels != IEnemy::Wheels::HeavyTracks && GameState::Instance().get_player_coins() >= params.cost) {
                    health = params.health;
                    GameState::Instance().player_coins_add(-params.cost);
                }
                else {
                    SoundManager::Instance().play(Sounds::SpikesBreaking);
                    set_health(health);
                    return;
                }
            }
		}
	}
    set_health(health);
}
