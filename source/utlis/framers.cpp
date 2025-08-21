#include "utils/framers.h"
#include "enemy_manager.h"

MedBlustFramer::MedBlustFramer() {
	sprite.setOrigin(8, 8);
	frames = 8;
}

void MedBlustFramer::on_frame(int frame) {
	if (frame < 4) {
		sprite.setTexture(EnemyManager::Instance().enemy_textures[EnemyTexturesID::MedBlustOfDestruction1]);
		sprite.setTextureRect(sf::IntRect(16 * (frame % 2), 16 * (frame / 2), 16, 16));
	}
	else {
		frame -= 4;
		sprite.setTexture(EnemyManager::Instance().enemy_textures[EnemyTexturesID::MedBlustOfDestruction2]);
		sprite.setTextureRect(sf::IntRect(16 * (frame % 2), 16 * (frame / 2), 16, 16));
	}
}

DoubleBlustFramer::DoubleBlustFramer() {
	sprite.setTexture(EnemyManager::Instance().enemy_textures[EnemyTexturesID::DoubleBlust]);
	sprite.setOrigin(8, 16);
	frames = 16;
}
void DoubleBlustFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 * (frame % 4), 32 * (frame / 4), 16, 32));
}

DenceBlustFramer::DenceBlustFramer() {
	frames = 12;
	sprite.setOrigin(8, 8);
	sprite.setTexture(EnemyManager::Instance().enemy_textures[EnemyTexturesID::Blusts16x16]);
}

void DenceBlustFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 * (frame % 8), 16 * (frame / 8), 16, 16));
}