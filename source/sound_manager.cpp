#include "sound_manager.h"
#include <numeric>


SoundManager::SoundManager() {
	sounds[Sounds::TankEngine].loadFromFile("sounds/tank.flac");
	sounds[Sounds::TwinGunShot].loadFromFile("sounds/twin_gun_shot.wav");
	sounds[Sounds::MedBlustOfDestruction].loadFromFile("sounds/med_blust_of_destruction.wav");
	sounds[Sounds::AntitankGunShot].loadFromFile("sounds/antitank_gun_shot.wav");
	sounds[Sounds::DenceBlust].loadFromFile("sounds/dence_blust.wav");
	sounds[Sounds::DoubleBlust].loadFromFile("sounds/double_blust.wav");
	sounds[Sounds::MiniGunShot].loadFromFile("sounds/minigun_shot.wav");
	sounds[Sounds::OverHeat].loadFromFile("sounds/overheat.wav");
}

void SoundManager::play(Sounds sound, int limit, float volume, float pitch) {
	int count = std::accumulate(m_requested.begin(), m_requested.end(), 0.0, [&](int buffer, const Request& req)->int {
		return buffer + static_cast<int>(req.id == sound);
	});
	if (count > limit)
		return;
	m_requested.push_back(Request{ sf::Sound(sounds[sound]), sound });
	m_requested.back().sound.setVolume(volume);
	m_requested.back().sound.setPitch(pitch);
	m_requested.back().sound.play();
}


void SoundManager::logic() {
	m_requested.remove_if([](const Request& sound) {
		return sound.sound.getStatus() == sf::SoundSource::Status::Stopped;
	});
}

sf::Sound SoundManager::get_sound(Sounds sound) {
	return sf::Sound(sounds[sound]);
}