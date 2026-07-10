#include "ResourceManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

ResourceManager& ResourceManager::instance() {
    static ResourceManager s_instance;
    return s_instance;
}

// ---- Texture ----
// Note: SFML 3 Texture still uses loadFromFile (NOT openFromFile)
sf::Texture* ResourceManager::loadTexture(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end())
        return it->second.get();

    auto tex = std::make_unique<sf::Texture>();
    if (tex->loadFromFile(path)) {
        sf::Texture* ptr = tex.get();
        m_textures[path] = std::move(tex);
        return ptr;
    }
    return nullptr;
}

// ---- Font ----
// Note: SFML 3 Font uses openFromFile (NOT loadFromFile)
sf::Font* ResourceManager::loadFont(const std::string& path) {
    auto it = m_fonts.find(path);
    if (it != m_fonts.end())
        return it->second.get();

    auto font = std::make_unique<sf::Font>();
    if (font->openFromFile(path)) {
        sf::Font* ptr = font.get();
        m_fonts[path] = std::move(font);
        return ptr;
    }
    return nullptr;
}

// ---- Shader ----
sf::Shader* ResourceManager::loadShader(const std::string& name, const std::string& fragPath) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end())
        return it->second.get();

    auto shader = std::make_unique<sf::Shader>();
    if (shader->loadFromFile(fragPath, sf::Shader::Type::Fragment)) {
        sf::Shader* ptr = shader.get();
        m_shaders[name] = std::move(shader);
        return ptr;
    }
    return nullptr;
}

// ---- SoundBuffer ----
sf::SoundBuffer* ResourceManager::loadSoundBuffer(const std::string& path) {
    auto it = m_sounds.find(path);
    if (it != m_sounds.end())
        return it->second.get();

    auto buf = std::make_unique<sf::SoundBuffer>();
    if (buf->loadFromFile(path)) {
        sf::SoundBuffer* ptr = buf.get();
        m_sounds[path] = std::move(buf);
        return ptr;
    }
    return nullptr;
}

// ---- clear ----
void ResourceManager::clear() {
    m_textures.clear();
    m_fonts.clear();
    m_shaders.clear();
    m_sounds.clear();
}
