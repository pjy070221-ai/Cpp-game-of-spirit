#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

// ╔══════════════════════════════════════════════════════════╗
// ║  ResourceManager — 游戏资源缓存（单例）                     ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】纹理、字体、着色器、音效的统一加载和缓存。          ║
// ║    每个文件只从磁盘加载一次，后续请求直接返回指针。          ║
// ║                                                          ║
// ║  【设计特点】                                              ║
// ║    1. 单例模式 — instance() 返回全局唯一实例                ║
// ║    2. 延迟加载 — loadXXX() 首次调用时读磁盘                 ║
// ║    3. 返回裸指针 — 所有权在内部 unique_ptr，外部不负责释放   ║
// ║    4. 失败返回 nullptr — 调用方需检查                       ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    - 所有需要加载资源的场景通过单例访问                      ║
// ║    - 目前 TitleScene 自己加载字体（因需要系统字体路径），     ║
// ║      后续可迁入 ResourceManager                            ║
// ║                                                          ║
// ║  【注意】当前项目中场景大多用系统字体或程序化生成纹理，       ║
// ║    所以 ResourceManager 使用频率不高。                      ║
// ║    打包正式版本时需要配合资源路径管理。                      ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝
class ResourceManager
{
public:
    static ResourceManager& instance()
    {
        static ResourceManager rm;
        return rm;
    }

    sf::Texture*     loadTexture(const std::string& path);
    sf::Texture*     getTexture(const std::string& path);
    sf::Font*        loadFont(const std::string& path);
    sf::Font*        getFont(const std::string& path);
    sf::Shader*      loadShader(const std::string& name, const std::string& fragPath, const std::string& vertPath = "");
    sf::Shader*      getShader(const std::string& name);
    sf::SoundBuffer* loadSoundBuffer(const std::string& path);
    sf::SoundBuffer* getSoundBuffer(const std::string& path);

    void clear();

private:
    ResourceManager() = default;

    std::unordered_map<std::string, std::unique_ptr<sf::Texture>>     m_textures;
    std::unordered_map<std::string, std::unique_ptr<sf::Font>>        m_fonts;
    std::unordered_map<std::string, std::unique_ptr<sf::Shader>>      m_shaders;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> m_sounds;
};
