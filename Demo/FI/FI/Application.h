#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "SceneManager.h"

// ╔══════════════════════════════════════════════════════════╗
// ║  Application — 应用程序主类                                ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】游戏顶层容器。持有窗口和场景管理器，驱动主循环。    ║
// ║    内置离屏渲染 + bloom 后处理管线。                       ║
// ║                                                          ║
// ║  【渲染管线】                                              ║
// ║    场景 → RenderTexture（离屏）→ bloom shader → 窗口      ║
// ║    所有场景自动获得发光后处理，无需逐个场景添加代码。        ║
// ║                                                          ║
// ║  【主循环三步骤】（run() 中每帧执行）                       ║
// ║    1. processEvents() — 轮询窗口事件并分发给当前场景       ║
// ║    2. update(dt)       — 更新场景逻辑 + 应用延迟场景切换   ║
// ║    3. render()         — 离屏渲染 → bloom → 显示          ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝
class Application
{
public:
    Application(const std::string& title, unsigned int width, unsigned int height);
    ~Application() = default;

    void run();

    sf::RenderWindow& getWindow()       { return m_window; }
    SceneManager&     getSceneManager() { return m_sceneManager; }
    float getDeltaTime() const          { return m_deltaTime; }

private:
    void processEvents();
    void update();
    void render();

    sf::RenderWindow m_window;
    SceneManager     m_sceneManager;
    sf::Clock        m_clock;
    float            m_deltaTime = 0.0f;

    // 注：预留 RenderTexture 成员用于后处理管线，SFML 3.1 下暂用直接渲染
};
