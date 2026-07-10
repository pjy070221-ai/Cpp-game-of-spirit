#pragma once

#include <string>
#include <unordered_map>
#include <variant>

using SettingVariant = std::variant<int, float, bool, std::string>;

class SettingsData {
public:
    // construct with config file path (default: settings.json in cwd)
    explicit SettingsData(const std::string& filePath = "settings.json");
    ~SettingsData();  // auto-save on destruction

    // no copy
    SettingsData(const SettingsData&) = delete;
    SettingsData& operator=(const SettingsData&) = delete;

    // generic template accessors (defined inline for template)
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

    // convenience accessors
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

private:
    bool loadFromFile();
    bool saveToFile() const;

    std::unordered_map<std::string, SettingVariant> m_settings;
    std::string m_configFilePath;
};
