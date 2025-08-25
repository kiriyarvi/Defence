#pragma once
#include <SFML/Audio.hpp>

#include <unordered_map>

#include <list>

enum class Sounds {
	TankEngine,
	TwinGunShot,
	MedBlustOfDestruction,
	AntitankGunShot,
	DenceBlust,
	DoubleBlust,
	MiniGunShot,
	OverHeat,
	MineBlast,
	Ricochet
};

class SoundManager {
public:
	static SoundManager& Instance() {
		static SoundManager instance; // Создаётся при первом вызове, потокобезопасно в C++11+
		return instance;
	}

	// Удаляем копирование и перемещение
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;
	SoundManager(SoundManager&&) = delete;
	SoundManager& operator=(SoundManager&&) = delete;

	void play(Sounds sound, int limit = 10, float volume = 100, float pitch = 1.);
	sf::Sound get_sound(Sounds sound);
	void logic();


	std::unordered_map<Sounds, sf::SoundBuffer> sounds;
private:
	SoundManager();
	struct Request {
		sf::Sound sound;
		Sounds id;
	};
	std::list<Request> m_requested;
};