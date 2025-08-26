#pragma once
#include <SFML/Graphics.hpp>

#include <unordered_map>


enum class Shader {
	GrayScale,
    Scroll
};

class ShaderManager {
public:
	// Получение единственного экземпляра
	static ShaderManager& Instance() {
		static ShaderManager instance; // Создаётся при первом вызове, потокобезопасно в C++11+
		return instance;
	}

	// Удаляем копирование и перемещение
	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;
	ShaderManager(ShaderManager&&) = delete;
	ShaderManager& operator=(ShaderManager&&) = delete;

	std::unordered_map<Shader, sf::Shader> shaders;
private:
	ShaderManager();
};
