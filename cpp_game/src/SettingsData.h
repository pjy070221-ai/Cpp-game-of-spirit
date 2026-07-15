#pragma once

#include <string>
#include <unordered_map>
#include <variant>

using SettingVariant = std::variant<int, float, bool, std::string>;

class SettingsData {
public:
    explicit SettingsData(const std::string& filePath = "settings.json");
    ~SettingsData();

    SettingsData(const SettingsData&) = delete;
    SettingsData& operator=(const SettingsData&) = delete;

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

    float getMasterVolume() const;
    void  setMasterVolume(float v);

    bool  getFullscreen() const;
    void  setFullscreen(bool v);

    bool getAutoPlay() const;
    void setAutoPlay(bool v);

    int getDifficulty() const;
    void setDifficulty(int v);
    void save();

private:
    bool loadFromFile();
    bool saveToFile() const;

    std::unordered_map<std::string, SettingVariant> m_settings;
    std::string m_configFilePath;
};
