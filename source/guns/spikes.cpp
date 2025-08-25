#include "guns/spikes.h"
#include "enemy_manager.h"


Spikes::Spikes(): params(ParamsManager::Instance().params.guns.spikes) 
{
	health = params.health;
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
		sprite.setTexture(TileMap::Instance().textures[TileTexture::SpikesCross]);
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
			sprite.setTexture(TileMap::Instance().textures[TileTexture::SpikesD]);
		}
		else { // прямая дорога
			sprite.setTexture(TileMap::Instance().textures[TileTexture::SpikesRight]);
			if (tile_id == 10) {
				rotation = 90;
			}
		}
	}
	else if (count == 3) {
		sprite.setTexture(TileMap::Instance().textures[TileTexture::SpikesT]);
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
	glm::vec2 pos(x_id * 32 + 16, y_id * 32 + 16);
	for (auto& enemy : EnemyManager::Instance().m_enemies) {
        if (enemy->wheels != IEnemy::Wheels::Wheels)
            continue;
		if (health <= 0)
			return;
		if (glm::length(pos - enemy->get_position()) < 0.2 * 32) {
			if(enemy->break_enemy(params.delay))
				--health;
		}
	}
}
