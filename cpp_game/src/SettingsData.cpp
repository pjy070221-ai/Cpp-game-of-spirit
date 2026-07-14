#include "SettingsData.h"
#include <fstream>
#include <sstream>
#include <filesystem>

SettingsData::SettingsData(const std::string& filePath)
    : m_configFilePath(filePath)
{
    // 确保所有键都有默认值（无论文件是否存在）
    m_settings["masterVolume"] = 0.8f;
    m_settings["noteSpeed"] = 5.0f;
    m_settings["fullscreen"] = false;
    m_settings["fpsLimit"] = 60.0f;
    m_settings["offset"] = 0.0f;
    m_settings["autoPlay"] = false;
    m_settings["difficulty"] = 1.0f;
    loadFromFile();  // 文件中的值覆盖默认值
}

SettingsData::~SettingsData() {
}

// ---- 便捷访问器 ----

float SettingsData::getMasterVolume() const {
    return get<float>("masterVolume", 0.8f);
}
void SettingsData::setMasterVolume(float v) {
    set<float>("masterVolume", v);
}

float SettingsData::getNoteSpeed() const {
    return get<float>("noteSpeed", 5.0f);
}
void SettingsData::setNoteSpeed(float v) {
    set<float>("noteSpeed", v);
}

bool SettingsData::getFullscreen() const {
    return get<bool>("fullscreen", false);
}
void SettingsData::setFullscreen(bool v) {
    set<bool>("fullscreen", v);
}

int SettingsData::getFpsLimit() const {
    auto it = m_settings.find("fpsLimit");
    if (it != m_settings.end()) {
        if (const auto* f = std::get_if<float>(&it->second)) return (int)*f;
        if (const auto* i = std::get_if<int>(&it->second)) return *i;
    }
    return 60;
}
void SettingsData::setFpsLimit(int v) {
    set<float>("fpsLimit", (float)v);
}

float SettingsData::getOffset() const {
    return get<float>("offset", 0.0f);
}
void SettingsData::setOffset(float ms) {
    set<float>("offset", ms);
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
        // 删除行首 UTF-8 BOM（如果存在）
        if (line.size() >= 3 && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
            line.erase(0, 3);
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        // 去除 Windows 行尾残留的 \r
        if (!val.empty() && val.back() == '\r') val.pop_back();

        if (key.empty()) continue;

        // 先 float 后 int（让纯整数也能存为 float，供 get<float> 读取）
        try { size_t pos = 0; float fv = std::stof(val, &pos); if (pos == val.size()) { set<float>(key, fv); continue; } } catch (...) {}
        // 再尝试解析为 int（兼容旧数据）
        try { size_t pos = 0; int iv = std::stoi(val, &pos); if (pos == val.size()) { set<int>(key, iv); continue; } } catch (...) {}
        // 布尔值
        if (val == "true")  { set<bool>(key, true);  continue; }
        if (val == "false") { set<bool>(key, false); continue; }
        // 都不匹配则作为字符串存储
        set<std::string>(key, val);
    }
    return true;
}

bool SettingsData::saveToFile() const {
    std::string tmpPath = m_configFilePath + ".tmp";
    std::ofstream file(tmpPath);
    if (!file.is_open()) return false;

    for (const auto& [key, val] : m_settings) {
        file << key << "=";
        if (const auto* i = std::get_if<int>(&val))          file << *i;
        else if (const auto* f = std::get_if<float>(&val))   file << *f;
        else if (const auto* b = std::get_if<bool>(&val))    file << (*b ? "true" : "false");
        else if (const auto* s = std::get_if<std::string>(&val)) file << *s;
        file << "\n";
    }
    file.close();
    std::error_code ec;
    std::filesystem::rename(tmpPath, m_configFilePath, ec);
    return !ec;
}

int SettingsData::getDifficulty() const {
    auto it = m_settings.find("difficulty");
    if (it != m_settings.end()) {
        if (const auto* f = std::get_if<float>(&it->second)) return (int)*f;
        if (const auto* i = std::get_if<int>(&it->second)) return *i;
    }
    return 1;
}
void SettingsData::setDifficulty(int v) { set<float>("difficulty", (float)v); }

void SettingsData::save() {
    saveToFile();
}
