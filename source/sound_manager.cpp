#include "sound_manager.h"

SoundManager::SoundManager() {
	sounds[Sounds::TankEngine].loadFromFile("sounds/tank.flac");
	sounds[Sounds::TwinGunShot].loadFromFile("sounds/twin_gun_shot.wav");
	sounds[Sounds::MedBlustOfDestruction].loadFromFile("sounds/med_blust_of_destruction.wav");
}

void SoundManager::play(Sounds sound) {
	m_requested.push_back(sf::Sound(sounds[sound]));
	m_requested.back().play();
}

void SoundManager::play_request(sf::Sound&& sound) {
	m_requested.push_back(std::move(sound));
	m_requested.back().play();
}

void SoundManager::logic() {
	m_requested.remove_if([](sf::Sound& sound) {
		return sound.getStatus() == sf::SoundSource::Status::Stopped;
	});
}

sf::Sound SoundManager::get_sound(Sounds sound) {
	return sf::Sound(sounds[sound]);
}