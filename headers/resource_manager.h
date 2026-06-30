#pragma once
#include <glm/vec2.hpp>
#include <vector>
#include <SFML/Graphics/Font.hpp>

class ResourceManager {
public:
    static ResourceManager& Instance() {
        static ResourceManager instance; 
        return instance;
    }

    // Удаляем копирование и перемещение
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    const std::vector<std::vector<glm::vec2>>& get_smoke_curl_noise() { return m_smoke_curl_noise; }
    sf::Font GOSTtypeA_font;
    sf::Font PixelSplitter_Bold_font;
private:
    ResourceManager();
private:
    std::vector<std::vector<glm::vec2>> m_smoke_curl_noise;
};
