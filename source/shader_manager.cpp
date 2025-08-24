#include "shader_manager.h"

ShaderManager::ShaderManager() {
	shaders[Shader::GrayScale].loadFromFile("shaders/grayscale.frag", sf::Shader::Fragment);
}