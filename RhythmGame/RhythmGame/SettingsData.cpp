#include "SettingsData.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

// ============================================================
// 构造函数
// ============================================================
SettingsData::SettingsData(const std::string& filePath)
    : configFilePath(filePath)
    , configVersion(1) {

    if (!loadFromFile()) {
        loadDefaults();
        saveToFile();
        std::cout << "[Settings] Created default config: " << filePath << std::endl;
    }
}

// ============================================================
// 加载默认值
// ============================================================
void SettingsData::loadDefaults() {
    // ===== 1. 音频设置 =====
    set<float>("masterVolume", 0.3f);
    set<float>("musicVolume", 0.4f);
    set<float>("sfxVolume", 0.3f);

    // ===== 2. 游戏设置 =====
    set<float>("noteSpeed", 3.0f);
    set<float>("offset", 0.0f);
    set<int>("noteScale", 100);
    set<bool>("showHitEffect", true);
    set<bool>("showKeyGuide", true);

    // ===== 3. 显示设置 =====
    set<int>("windowWidth", 1280);
    set<int>("windowHeight", 720);
    set<bool>("fullscreen", false);
    set<int>("fpsLimit", 144);
    set<bool>("vsync", false);

    // ===== 4. 按键映射 =====
    set<std::map<std::string, sf::Keyboard::Key>>("keyBindings", getDefaultKeyBindings());

    // 版本号
    set<int>("configVersion", configVersion);
}

// ============================================================
// 默认按键映射
// ============================================================
std::map<std::string, sf::Keyboard::Key> SettingsData::getDefaultKeyBindings() {
    std::map<std::string, sf::Keyboard::Key> keys;

    // 4K 默认：D F J K (SFML 3.1.0 使用 sf::Keyboard::Key::)
    keys["key1"] = sf::Keyboard::Key::D;
    keys["key2"] = sf::Keyboard::Key::F;
    keys["key3"] = sf::Keyboard::Key::J;
    keys["key4"] = sf::Keyboard::Key::K;

    // 功能键
    keys["pause"] = sf::Keyboard::Key::P;
    keys["restart"] = sf::Keyboard::Key::R;
    keys["menuOpen"] = sf::Keyboard::Key::Escape;
    keys["select"] = sf::Keyboard::Key::Enter;
    keys["navigateUp"] = sf::Keyboard::Key::Up;
    keys["navigateDown"] = sf::Keyboard::Key::Down;
    keys["navigateLeft"] = sf::Keyboard::Key::Left;
    keys["navigateRight"] = sf::Keyboard::Key::Right;

    return keys;
}

// ============================================================
// 保存到文件
// ============================================================
bool SettingsData::saveToFile() const {
    try {
        json j;

        for (const auto& [key, value] : settings) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, sf::Keyboard::Key>) {
                    j[key] = static_cast<int>(arg);
                }
                else if constexpr (std::is_same_v<T, std::map<std::string, sf::Keyboard::Key>>) {
                    json keyMap;
                    for (const auto& [action, keyCode] : arg) {
                        keyMap[action] = static_cast<int>(keyCode);
                    }
                    j[key] = keyMap;
                }
                else {
                    j[key] = arg;
                }
                }, value);
        }

        std::ofstream file(configFilePath);
        if (!file.is_open()) {
            std::cerr << "[Settings] Failed to open file for writing" << std::endl;
            return false;
        }

        file << j.dump(4);
        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "[Settings] Save error: " << e.what() << std::endl;
        return false;
    }
}

// ============================================================
// 从文件加载
// ============================================================
bool SettingsData::loadFromFile() {
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        return false;
    }

    try {
        json j;
        file >> j;

        for (auto& [key, value] : j.items()) {
            if (key == "keyBindings") {
                std::map<std::string, sf::Keyboard::Key> keyMap;
                for (auto& [action, keyCode] : value.items()) {
                    keyMap[action] = static_cast<sf::Keyboard::Key>(keyCode.get<int>());
                }
                set(key, keyMap);
            }
            else if (value.is_number_integer()) {
                set(key, value.get<int>());
            }
            else if (value.is_number_float()) {
                set(key, value.get<float>());
            }
            else if (value.is_boolean()) {
                set(key, value.get<bool>());
            }
            else if (value.is_string()) {
                set(key, value.get<std::string>());
            }
        }

        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "[Settings] Load error: " << e.what() << std::endl;
        return false;
    }
}

// ============================================================
// 重置
// ============================================================
void SettingsData::resetToDefaults() {
    settings.clear();
    loadDefaults();
    saveToFile();
}

// ============================================================
// 辅助方法
// ============================================================
bool SettingsData::hasKey(const std::string& key) const {
    return settings.find(key) != settings.end();
}

void SettingsData::removeKey(const std::string& key) {
    settings.erase(key);
}

// ============================================================
// 获取按键名称
// ============================================================
std::string SettingsData::getKeyName(sf::Keyboard::Key key) {
    switch (key) {
    case sf::Keyboard::Key::A: return "A";
    case sf::Keyboard::Key::B: return "B";
    case sf::Keyboard::Key::C: return "C";
    case sf::Keyboard::Key::D: return "D";
    case sf::Keyboard::Key::E: return "E";
    case sf::Keyboard::Key::F: return "F";
    case sf::Keyboard::Key::G: return "G";
    case sf::Keyboard::Key::H: return "H";
    case sf::Keyboard::Key::I: return "I";
    case sf::Keyboard::Key::J: return "J";
    case sf::Keyboard::Key::K: return "K";
    case sf::Keyboard::Key::L: return "L";
    case sf::Keyboard::Key::M: return "M";
    case sf::Keyboard::Key::N: return "N";
    case sf::Keyboard::Key::O: return "O";
    case sf::Keyboard::Key::P: return "P";
    case sf::Keyboard::Key::Q: return "Q";
    case sf::Keyboard::Key::R: return "R";
    case sf::Keyboard::Key::S: return "S";
    case sf::Keyboard::Key::T: return "T";
    case sf::Keyboard::Key::U: return "U";
    case sf::Keyboard::Key::V: return "V";
    case sf::Keyboard::Key::W: return "W";
    case sf::Keyboard::Key::X: return "X";
    case sf::Keyboard::Key::Y: return "Y";
    case sf::Keyboard::Key::Z: return "Z";
    case sf::Keyboard::Key::Space: return "Space";
    case sf::Keyboard::Key::Enter: return "Enter";
    case sf::Keyboard::Key::Escape: return "Esc";
    case sf::Keyboard::Key::Left: return "Left";
    case sf::Keyboard::Key::Right: return "Right";
    case sf::Keyboard::Key::Up: return "Up";
    case sf::Keyboard::Key::Down: return "Down";
    default: return "Unknown";
    }
}

// ============================================================
// 便捷访问器实现
// ============================================================

// ----- 音频 -----
float SettingsData::getMasterVolume() const { return get<float>("masterVolume", 0.8f); }
void SettingsData::setMasterVolume(float v) { set<float>("masterVolume", std::clamp(v, 0.0f, 1.0f)); }

float SettingsData::getMusicVolume() const { return get<float>("musicVolume", 0.5f); }
void SettingsData::setMusicVolume(float v) { set<float>("musicVolume", std::clamp(v, 0.0f, 1.0f)); }

float SettingsData::getSfxVolume() const { return get<float>("sfxVolume", 0.7f); }
void SettingsData::setSfxVolume(float v) { set<float>("sfxVolume", std::clamp(v, 0.0f, 1.0f)); }

// ----- 游戏 -----
float SettingsData::getNoteSpeed() const { return get<float>("noteSpeed", 5.0f); }
void SettingsData::setNoteSpeed(float v) { set<float>("noteSpeed", std::clamp(v, 1.0f, 10.0f)); }

float SettingsData::getOffset() const { return get<float>("offset", 0.0f); }
void SettingsData::setOffset(float v) { set<float>("offset", std::clamp(v, -100.0f, 100.0f)); }

int SettingsData::getNoteScale() const { return get<int>("noteScale", 100); }
void SettingsData::setNoteScale(int v) { set<int>("noteScale", std::clamp(v, 80, 120)); }

bool SettingsData::getShowHitEffect() const { return get<bool>("showHitEffect", true); }
void SettingsData::setShowHitEffect(bool v) { set<bool>("showHitEffect", v); }

bool SettingsData::getShowKeyGuide() const { return get<bool>("showKeyGuide", true); }
void SettingsData::setShowKeyGuide(bool v) { set<bool>("showKeyGuide", v); }

// ----- 显示 -----
sf::Vector2u SettingsData::getWindowSize() const {
    return sf::Vector2u(
        get<int>("windowWidth", 1280),
        get<int>("windowHeight", 720)
    );
}
void SettingsData::setWindowSize(sf::Vector2u size) {
    set<int>("windowWidth", static_cast<int>(size.x));
    set<int>("windowHeight", static_cast<int>(size.y));
}
void SettingsData::setWindowSize(int width, int height) {
    set<int>("windowWidth", width);
    set<int>("windowHeight", height);
}

bool SettingsData::getFullscreen() const { return get<bool>("fullscreen", false); }
void SettingsData::setFullscreen(bool v) { set<bool>("fullscreen", v); }

int SettingsData::getFpsLimit() const { return get<int>("fpsLimit", 144); }
void SettingsData::setFpsLimit(int v) { set<int>("fpsLimit", std::max(0, v)); }

bool SettingsData::getVsync() const { return get<bool>("vsync", false); }
void SettingsData::setVsync(bool v) { set<bool>("vsync", v); }

// ----- 按键映射 -----
sf::Keyboard::Key SettingsData::getKeyBinding(const std::string& action) const {
    auto bindings = get<std::map<std::string, sf::Keyboard::Key>>("keyBindings", {});
    auto it = bindings.find(action);
    if (it != bindings.end()) {
        return it->second;
    }
    return sf::Keyboard::Key::Unknown;
}

void SettingsData::setKeyBinding(const std::string& action, sf::Keyboard::Key key) {
    auto bindings = get<std::map<std::string, sf::Keyboard::Key>>("keyBindings", {});
    bindings[action] = key;
    set<std::map<std::string, sf::Keyboard::Key>>("keyBindings", bindings);
}

std::map<std::string, sf::Keyboard::Key> SettingsData::getAllKeyBindings() const {
    return get<std::map<std::string, sf::Keyboard::Key>>("keyBindings", {});
}

void SettingsData::setAllKeyBindings(const std::map<std::string, sf::Keyboard::Key>& bindings) {
    set<std::map<std::string, sf::Keyboard::Key>>("keyBindings", bindings);
}