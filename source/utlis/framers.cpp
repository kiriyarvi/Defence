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

SteamFramer::SteamFramer() {
	frames = 4;
	sprite.setTexture(TileMap::Instance().textures[TileTexture::MiniGunEquipment]);
	sprite.setOrigin(1.5, 4);
}

void SteamFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16, 22 + frame, 3, 4));
}


MinigunHitFramer::MinigunHitFramer() {
	frames = 2;
	sprite.setTexture(TileMap::Instance().textures[TileTexture::MiniGunEquipment]);
	sprite.setOrigin(1.5, 1.5);
}

void MinigunHitFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 + 3 * frame, 19, 3, 3));
}


MinigunShootFramer::MinigunShootFramer() {
	frames = 4;
	sprite.setTexture(TileMap::Instance().textures[TileTexture::MiniGunEquipment]);
	sprite.setOrigin(0, 1.5);
}

void MinigunShootFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 + 3 * frame, 16, 3, 3));
}


MinigunReboundFramer::MinigunReboundFramer() {
	frames = 4;
	sprite.setTexture(TileMap::Instance().textures[TileTexture::MiniGunEquipment]);
	sprite.setOrigin(0, 1.5);
}

void MinigunReboundFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(19 + 2 * frame, 22, 2, 3));
}

MineBlast::MineBlast() {
	frames = 14;
	sprite.setTexture(TileMap::Instance().textures[TileTexture::MineBlast]);
	sprite.setOrigin(8,8);
}

void MineBlast::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 * (frame % 4), 16 * (frame / 4), 16, 16));
}