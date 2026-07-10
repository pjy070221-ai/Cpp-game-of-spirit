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

// ---- convenience accessors ----

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
    return get<int>("fpsLimit", 60);
}
void SettingsData::setFpsLimit(int v) {
    set<int>("fpsLimit", v);
}

float SettingsData::getOffset() const {
    return get<float>("offset", 0.0f);
}
void SettingsData::setOffset(float ms) {
    set<float>("offset", ms);
}

// ---- file I/O ----
// format: key=value  (one per line)

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

        // try int first
        try { size_t pos = 0; int iv = std::stoi(val, &pos); if (pos == val.size()) { set<int>(key, iv); continue; } } catch (...) {}
        // try float
        try { size_t pos = 0; float fv = std::stof(val, &pos); if (pos == val.size()) { set<float>(key, fv); continue; } } catch (...) {}
        // bool
        if (val == "true")  { set<bool>(key, true);  continue; }
        if (val == "false") { set<bool>(key, false); continue; }
        // string fallback
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
