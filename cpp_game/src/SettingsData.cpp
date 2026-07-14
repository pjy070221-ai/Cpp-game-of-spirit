#include "SettingsData.h"
#include <fstream>
#include <sstream>

SettingsData::SettingsData(const std::string& filePath)
    : m_configFilePath(filePath)
{
    loadFromFile();
}

SettingsData::~SettingsData() {
    saveToFile();
}

// ---- 便捷访问器 ----

float SettingsData::getMasterVolume() const {
    return get<float>("masterVolume", 0.8f);
}
void SettingsData::setMasterVolume(float v) {
    set<float>("masterVolume", v);
}




bool SettingsData::getFullscreen() const {
    return get<bool>("fullscreen", false);
}
void SettingsData::setFullscreen(bool v) {
    set<bool>("fullscreen", v);
}







bool SettingsData::getAutoPlay() const { return get<bool>("autoPlay", false); }
void SettingsData::setAutoPlay(bool v) { set<bool>("autoPlay", v); }

// ---- 文件读写 ----
// 格式：key=value（每行一个）

bool SettingsData::loadFromFile() {
    std::ifstream file(m_configFilePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        if (key.empty()) continue;

        // 先尝试解析为 int
        try { size_t pos = 0; int iv = std::stoi(val, &pos); if (pos == val.size()) { set<int>(key, iv); continue; } } catch (...) {}
        // 再尝试解析为 float
        try { size_t pos = 0; float fv = std::stof(val, &pos); if (pos == val.size()) { set<float>(key, fv); continue; } } catch (...) {}
        // 布尔值
        if (val == "true")  { set<bool>(key, true);  continue; }
        if (val == "false") { set<bool>(key, false); continue; }
        // 都不匹配则作为字符串存储
        set<std::string>(key, val);
    }
    return true;
}

bool SettingsData::saveToFile() const {
    std::ofstream file(m_configFilePath);
    if (!file.is_open()) return false;

    for (const auto& [key, val] : m_settings) {
        file << key << "=";
        if (const auto* i = std::get_if<int>(&val))          file << *i;
        else if (const auto* f = std::get_if<float>(&val))   file << *f;
        else if (const auto* b = std::get_if<bool>(&val))    file << (*b ? "true" : "false");
        else if (const auto* s = std::get_if<std::string>(&val)) file << *s;
        file << "\n";
    }
    return true;
}

int SettingsData::getDifficulty() const { return get<int>("difficulty", 1); }
void SettingsData::setDifficulty(int v) { set<int>("difficulty", v); }

void SettingsData::save() {
    saveToFile();
}
