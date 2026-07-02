#include "ResourceManager.h"

sf::Texture* ResourceManager::loadTexture(const std::string& path)
{
    auto it = m_textures.find(path);
    if (it != m_textures.end())
        return it->second.get();

    auto texture = std::make_unique<sf::Texture>();
    if (!texture->loadFromFile(path))
        return nullptr;

    m_textures[path] = std::move(texture);
    return m_textures[path].get();
}

sf::Texture* ResourceManager::getTexture(const std::string& path)
{
    auto it = m_textures.find(path);
    return (it != m_textures.end()) ? it->second.get() : nullptr;
}

sf::Font* ResourceManager::loadFont(const std::string& path)
{
    auto it = m_fonts.find(path);
    if (it != m_fonts.end())
        return it->second.get();

    auto font = std::make_unique<sf::Font>();
    if (!font->openFromFile(path))
        return nullptr;

    m_fonts[path] = std::move(font);
    return m_fonts[path].get();
}

sf::Font* ResourceManager::getFont(const std::string& path)
{
    auto it = m_fonts.find(path);
    return (it != m_fonts.end()) ? it->second.get() : nullptr;
}

sf::Shader* ResourceManager::loadShader(
    const std::string& name,
    const std::string& fragPath,
    const std::string& vertPath)
{
    auto it = m_shaders.find(name);
    if (it != m_shaders.end())
        return it->second.get();

    auto shader = std::make_unique<sf::Shader>();
    if (vertPath.empty())
    {
        if (!shader->loadFromFile(fragPath, sf::Shader::Type::Fragment))
            return nullptr;
    }
    else
    {
        if (!shader->loadFromFile(vertPath, fragPath))
            return nullptr;
    }

    m_shaders[name] = std::move(shader);
    return m_shaders[name].get();
}

sf::Shader* ResourceManager::getShader(const std::string& name)
{
    auto it = m_shaders.find(name);
    return (it != m_shaders.end()) ? it->second.get() : nullptr;
}

sf::SoundBuffer* ResourceManager::loadSoundBuffer(const std::string& path)
{
    auto it = m_sounds.find(path);
    if (it != m_sounds.end())
        return it->second.get();

    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(path))
        return nullptr;

    m_sounds[path] = std::move(buffer);
    return m_sounds[path].get();
}

sf::SoundBuffer* ResourceManager::getSoundBuffer(const std::string& path)
{
    auto it = m_sounds.find(path);
    return (it != m_sounds.end()) ? it->second.get() : nullptr;
}

void ResourceManager::clear()
{
    m_textures.clear();
    m_fonts.clear();
    m_shaders.clear();
    m_sounds.clear();
}
