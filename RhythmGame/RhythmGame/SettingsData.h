#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <variant>
#include <map>
#include <string>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

// 支持的数据类型
using SettingVariant = std::variant<
    int,
    float,
    bool,
    std::string,
    sf::Keyboard::Key,
    std::map<std::string, sf::Keyboard::Key>
>;

// ============================================================
// SettingsData 基础类
// ============================================================
class SettingsData {
public:
    SettingsData(const std::string& filePath = "settings.json");
    ~SettingsData() { saveToFile(); }

    // ===== 加载/保存 =====
    bool loadFromFile();
    bool saveToFile() const;
    void resetToDefaults();

    // ===== 通用存取接口（模板直接写头文件） =====
    template<typename T>
    T get(const std::string& key, T defaultValue) const {
        auto it = settings.find(key);
        if (it != settings.end() && std::holds_alternative<T>(it->second)) {
            return std::get<T>(it->second);
        }
        return defaultValue;
    }

    template<typename T>
    void set(const std::string& key, T value) {
        settings[key] = value;
    }

    bool hasKey(const std::string& key) const;
    void removeKey(const std::string& key);

    // ============================================================
    // 1. 音频设置 (3项)
    // ============================================================
    float getMasterVolume() const;
    void setMasterVolume(float volume);
    float getMusicVolume() const;
    void setMusicVolume(float volume);
    float getSfxVolume() const;
    void setSfxVolume(float volume);

    // ============================================================
    // 2. 游戏设置 (5项)
    // ============================================================
    float getNoteSpeed() const;
    void setNoteSpeed(float speed);
    float getOffset() const;
    void setOffset(float offset);
    int getNoteScale() const;
    void setNoteScale(int scale);
    bool getShowHitEffect() const;
    void setShowHitEffect(bool enabled);
    bool getShowKeyGuide() const;
    void setShowKeyGuide(bool enabled);

    // ============================================================
    // 3. 显示设置 (5项)
    // ============================================================
    sf::Vector2u getWindowSize() const;
    void setWindowSize(sf::Vector2u size);
    void setWindowSize(int width, int height);
    bool getFullscreen() const;
    void setFullscreen(bool fullscreen);
    int getFpsLimit() const;
    void setFpsLimit(int fps);
    bool getVsync() const;
    void setVsync(bool enabled);

    // ============================================================
    // 4. 按键映射 (4K + 功能键)
    // ============================================================
    sf::Keyboard::Key getKeyBinding(const std::string& action) const;
    void setKeyBinding(const std::string& action, sf::Keyboard::Key key);
    std::map<std::string, sf::Keyboard::Key> getAllKeyBindings() const;
    void setAllKeyBindings(const std::map<std::string, sf::Keyboard::Key>& bindings);

    // 获取默认按键映射
    static std::map<std::string, sf::Keyboard::Key> getDefaultKeyBindings();

    // ============================================================
    // 5. 工具方法
    // ============================================================
    static std::string getKeyName(sf::Keyboard::Key key);

private:
    void loadDefaults();

    std::unordered_map<std::string, SettingVariant> settings;
    std::string configFilePath;
    int configVersion;
};

// ===== nlohmann/json 序列化支持 =====
namespace nlohmann {
    template<>
    struct adl_serializer<sf::Keyboard::Key> {
        static void to_json(json& j, const sf::Keyboard::Key& key) {
            j = static_cast<int>(key);
        }
        static void from_json(const json& j, sf::Keyboard::Key& key) {
            key = static_cast<sf::Keyboard::Key>(j.get<int>());
        }
    };
}