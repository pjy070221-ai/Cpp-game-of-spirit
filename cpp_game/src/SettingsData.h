#pragma once

#include <string>
#include <unordered_map>
#include <variant>

using SettingVariant = std::variant<int, float, bool, std::string>;

class SettingsData {
public:
    // 构造时指定配置文件路径（默认：当前目录 settings.json）
    explicit SettingsData(const std::string& filePath = "settings.json");
    ~SettingsData();  // 析构时自动保存

    // 禁止拷贝
    SettingsData(const SettingsData&) = delete;
    SettingsData& operator=(const SettingsData&) = delete;

    // 泛型模板访问器（在头文件中内联实现）
    template<typename T>
    T get(const std::string& key, T defaultValue) const {
        auto it = m_settings.find(key);
        if (it != m_settings.end()) {
            if (const auto* val = std::get_if<T>(&it->second))
                return *val;
        }
        return defaultValue;
    }

    template<typename T>
    void set(const std::string& key, T value) {
        m_settings[key] = value;
    }

    // 便捷访问器
    float getMasterVolume() const;
    void  setMasterVolume(float v);

    float getNoteSpeed() const;
    void  setNoteSpeed(float v);

    bool  getFullscreen() const;
    void  setFullscreen(bool v);

    int   getFpsLimit() const;
    void  setFpsLimit(int v);

    float getOffset() const;
    void  setOffset(float ms);

    bool getAutoPlay() const;
    void setAutoPlay(bool v);

private:
    bool loadFromFile();
    bool saveToFile() const;

    std::unordered_map<std::string, SettingVariant> m_settings;
    std::string m_configFilePath;
};
