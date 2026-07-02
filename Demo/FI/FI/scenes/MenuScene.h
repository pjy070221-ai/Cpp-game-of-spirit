#pragma once
#include "IScene.h"
#include "../core/ParticleSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ╔══════════════════════════════════════════════════════════╗
// ║  MenuScene — 主菜单场景                                    ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】标题画面之后的主菜单，提供选曲入口。              ║
// ║                                                          ║
// ║  【视觉】                                                  ║
// ║    - 暗蓝紫渐变背景 + 星空粒子（与 TitleScene 风格一致）    ║
// ║    - 菜单项从右侧 staggered 滑入（easeOutBack 回弹）       ║
// ║    - 选中项高亮白 + 蓝色光晕描边 + 三角指示器平滑移动       ║
// ║                                                          ║
// ║  【交互】                                                  ║
// ║    - ↑↓ 切换选中项（循环）                                 ║
// ║    - Enter 确认选择                                       ║
// ║    - ESC 返回 TitleScene                                 ║
// ║                                                          ║
// ║  【与其他 Scene 的关系】                                    ║
// ║    - TitleScene → Enter → MenuScene                      ║
// ║    - MenuScene → START GAME → PackScene                  ║
// ║    - MenuScene → ESC → TitleScene                        ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝
class MenuScene : public IScene
{
public:
    MenuScene();
    ~MenuScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    // ── 菜单项 ──
    struct MenuItem {
        std::string label;
        float targetX;       // 最终 X 位置
        float targetY;       // 最终 Y 位置
        float currentX;      // 动画当前 X
        float entryDelay;    // 入场延迟（秒）
    };
    std::vector<MenuItem> m_items;
    int m_selectedIndex = 0;
    int m_previousIndex = 0;      // 用于指示器平滑过渡

    // ── 指示器 ──
    float m_indicatorY = 0.f;     // ▶ 当前 Y
    float m_indicatorTargetY = 0.f; // ▶ 目标 Y（选中项位置）

    // ── 字体 & 文本 ──
    sf::Font m_font;
    bool m_fontLoaded = false;
    sf::Text m_titleText;         // 顶部 "ABYSSAL BEAT"
    std::vector<sf::Text> m_itemTexts;  // 每个菜单项的 sf::Text
    sf::Text m_hintText;          // 底部提示

    // ── 入场动画 ──
    float m_elapsedTime = 0.f;

    // ── 背景 ──
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    ParticleSystem m_stars;

    // ── 方法 ──
    void buildBackground();
    void buildMenuItems();
    void updateItemPositions();
    void updateIndicator(float dt);
};
