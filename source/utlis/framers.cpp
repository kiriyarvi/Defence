#include "utils/framers.h"
#include "texture_manager.h"


MedBlustFramer::MedBlustFramer() {
	sprite.setOrigin(8, 8);
	frames = 8;
}

void MedBlustFramer::on_frame(int frame) {
	if (frame < 4) {
		sprite.setTexture(TextureManager::Instance().textures[TextureID::MedBlustOfDestruction1]);
		sprite.setTextureRect(sf::IntRect(16 * (frame % 2), 16 * (frame / 2), 16, 16));
	}
	else {
		frame -= 4;
		sprite.setTexture(TextureManager::Instance().textures[TextureID::MedBlustOfDestruction2]);
		sprite.setTextureRect(sf::IntRect(16 * (frame % 2), 16 * (frame / 2), 16, 16));
	}
}

DoubleBlustFramer::DoubleBlustFramer() {
	sprite.setTexture(TextureManager::Instance().textures[TextureID::DoubleBlust]);
	sprite.setOrigin(8, 16);
	frames = 16;
}
void DoubleBlustFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 * (frame % 4), 32 * (frame / 4), 16, 32));
}

DenceBlustFramer::DenceBlustFramer() {
	frames = 12;
	sprite.setOrigin(8, 8);
	sprite.setTexture(TextureManager::Instance().textures[TextureID::Blusts16x16]);
}

void DenceBlustFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 * (frame % 8), 16 * (frame / 8), 16, 16));
}

SteamFramer::SteamFramer() {
	frames = 4;
	sprite.setTexture(TextureManager::Instance().textures[TextureID::MiniGunEquipment]);
	sprite.setOrigin(1.5, 4);
}

void SteamFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16, 22 + frame, 3, 4));
}


MinigunHitFramer::MinigunHitFramer() {
	frames = 2;
	sprite.setTexture(TextureManager::Instance().textures[TextureID::MiniGunEquipment]);
	sprite.setOrigin(1.5, 1.5);
}

void MinigunHitFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 + 3 * frame, 19, 3, 3));
}


MinigunShootFramer::MinigunShootFramer() {
	frames = 4;
	sprite.setTexture(TextureManager::Instance().textures[TextureID::MiniGunEquipment]);
	sprite.setOrigin(0, 1.5);
}

void MinigunShootFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 + 3 * frame, 16, 3, 3));
}


MinigunReboundFramer::MinigunReboundFramer() {
	frames = 4;
	sprite.setTexture(TextureManager::Instance().textures[TextureID::MiniGunEquipment]);
	sprite.setOrigin(0, 1.5);
}

void MinigunReboundFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(19 + 2 * frame, 22, 2, 3));
}

MineBlastFramer::MineBlastFramer() {
	frames = 14;
	sprite.setTexture(TextureManager::Instance().textures[TextureID::MineBlast]);
	sprite.setOrigin(8,8);
}

void MineBlastFramer::on_frame(int frame) {
	sprite.setTextureRect(sf::IntRect(16 * (frame % 4), 16 * (frame / 4), 16, 16));
}

PickupBlastFramer::PickupBlastFramer() {
    frames = 12;
    sprite.setOrigin(8, 8);
    sprite.setTexture(TextureManager::Instance().textures[TextureID::Blusts16x16]);
}

void PickupBlastFramer::on_frame(int frame) {
    frame += 16;
    sprite.setTextureRect(sf::IntRect(16 * (frame % 8), 16 * (frame / 8), 16, 16));
}

CruiserIFireFramer::CruiserIFireFramer() {
    frames = 5;
    sprite.setOrigin(4, 8);
    sprite.setTexture(TextureManager::Instance().textures[TextureID::CruiserISupplementary]);
}

void CruiserIFireFramer::on_frame(int frame) {
    if (frame < 4)
        sprite.setTextureRect(sf::IntRect(16 + 8 * (frame % 2), 8 * (frame / 2), 8, 8));
    else
        sprite.setTextureRect(sf::IntRect(0, 16, 8, 8));
}


CruiserIBlustFramer::CruiserIBlustFramer() {
    frames = 19;
    sprite.setTexture(TextureManager::Instance().textures[TextureID::CruiserIBlust]);
}

void CruiserIBlustFramer::on_frame(int frame) {
    if (frame < 15) {
        sprite.setOrigin(16, 16);
        sprite.setTextureRect(sf::IntRect(32 * (frame % 4), 32 * (frame / 4), 32, 32));
    }
    else {
        frame -= 15;
        sprite.setOrigin(8, 8);
        sprite.setTextureRect(sf::IntRect(32 * 3 + 16 * (frame % 2), 32 * 3 + 16 * (frame / 2), 16, 16));
    }
}
