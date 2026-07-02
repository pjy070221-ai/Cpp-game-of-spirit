#pragma once
#include "IScene.h"
#include "../core/ParticleSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ╔══════════════════════════════════════════════════════════╗
// ║  SettingsScene — 设置界面                                  ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】调节音符流速、音量、音效、延迟。                  ║
// ║    设置值存入 GameSettings 全局单例。                      ║
// ║                                                          ║
// ║  【入口】PackScene 按 S 键                                 ║
// ║  【出口】ESC → PackScene                                  ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝

struct GameSettings {
    float noteSpeed  = 2.0f;    // 流速倍数 (1.0~3.0)
    float musicVol   = 80.f;    // 音乐音量 (0~100)
    float sfxVol     = 70.f;    // 音效音量 (0~100)
    float latency    = 0.f;     // 延迟偏移 (ms, -200~200)

    static GameSettings& inst() { static GameSettings s; return s; }
};

class SettingsScene : public IScene
{
public:
    SettingsScene();
    ~SettingsScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    struct SettingItem {
        std::string label;
        std::string valueStr;
        float* valuePtr = nullptr;
        float minVal = 0.f, maxVal = 100.f, step = 1.f;
        std::string unit;
        void updateString();
    };
    std::vector<SettingItem> m_items;
    int m_selectedIndex = 0;

    float m_elapsed = 0.f;
    sf::Font m_font;
    sf::Text m_titleText;
    sf::Text m_hintText;
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    ParticleSystem m_stars;

    // 预创建 UI 对象（避免每帧光栅化）
    std::vector<sf::Text> m_labels;
    std::vector<sf::Text> m_values;
    std::vector<sf::RectangleShape> m_tracks;
    std::vector<sf::RectangleShape> m_fills;
    sf::CircleShape m_dot{ 7.f };

    void buildBackground();
    void buildItems();
    std::string valueDisplay(const SettingItem& item) const;
};
