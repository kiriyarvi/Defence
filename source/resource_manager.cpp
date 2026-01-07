#include <resource_manager.h>

#include <fstream>

ResourceManager::ResourceManager() {
    std::ifstream file("noises/curl_noise.bin", std::ios::binary);
    uint32_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));

    m_smoke_curl_noise.resize(size, std::vector<glm::vec2>(size));
    for (auto& row : m_smoke_curl_noise) {
        file.read(reinterpret_cast<char*>(row.data()),
            row.size() * sizeof(glm::vec2));
    }
}
