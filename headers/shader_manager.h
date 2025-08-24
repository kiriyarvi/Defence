#pragma once
#include <SFML/Graphics.hpp>

#include <unordered_map>


enum class Shader {
	GrayScale
};

class ShaderManager {
public:
	// ��������� ������������� ����������
	static ShaderManager& Instance() {
		static ShaderManager instance; // �������� ��� ������ ������, ��������������� � C++11+
		return instance;
	}

	// ������� ����������� � �����������
	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;
	ShaderManager(ShaderManager&&) = delete;
	ShaderManager& operator=(ShaderManager&&) = delete;

	std::unordered_map<Shader, sf::Shader> shaders;
private:
	ShaderManager();
};