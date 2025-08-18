#include <SFML/Audio.hpp>

#include <unordered_map>

#include <list>

enum class Sounds {
	TankEngine,
	TwinGunShot,
	MedBlustOfDestruction
};

class SoundManager {
public:
	static SoundManager& Instance() {
		static SoundManager instance; // �������� ��� ������ ������, ��������������� � C++11+
		return instance;
	}

	// ������� ����������� � �����������
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;
	SoundManager(SoundManager&&) = delete;
	SoundManager& operator=(SoundManager&&) = delete;

	void play(Sounds sound);
	void play_request(sf::Sound&& sound);
	sf::Sound get_sound(Sounds sound);
	void logic();


	std::unordered_map< Sounds, sf::SoundBuffer> sounds;
private:
	SoundManager();
	std::list<sf::Sound> m_requested;
};