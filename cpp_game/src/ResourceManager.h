#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace sf {
    class Texture;
    class Font;
    class Shader;
    class SoundBuffer;
}

class ResourceManager {
public:
    static ResourceManager& instance();

    // texture loading
    sf::Texture* loadTexture(const std::string& path);

    // font loading
    sf::Font* loadFont(const std::string& path);

    // shader loading (vertex/fragment)
    sf::Shader* loadShader(const std::string& name, const std::string& fragPath);

    // sound buffer loading
    sf::SoundBuffer* loadSoundBuffer(const std::string& path);

    // clear all cached resources
    void clear();

private:
    ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::unordered_map<std::string, std::unique_ptr<sf::Texture>>     m_textures;
    std::unordered_map<std::string, std::unique_ptr<sf::Font>>        m_fonts;
    std::unordered_map<std::string, std::unique_ptr<sf::Shader>>      m_shaders;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> m_sounds;
};
