#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

// 前向声明，避免循环依赖
class SceneManager;

// ╔══════════════════════════════════════════════════════════╗
// ║  IScene — 场景抽象基类                                    ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】所有游戏画面的统一接口。一个场景 = 一整个画面状态   ║
// ║    （标题、菜单、曲包、演奏、结算）。                      ║
// ║                                                          ║
// ║  【设计特点】                                              ║
// ║    1. 栈式管理 — SceneManager 用 std::vector 模拟栈，       ║
// ║       pushScene() 暂停当前场景并推入新场景，                ║
// ║       popScene() 销毁当前并回到上一场景。                   ║
// ║    2. 延迟切换 — requestPush/Pop/Replace 不会立刻执行，     ║
// ║       而是挂起到帧末。这防止了在 update() 中 delete this    ║
// ║       导致的崩溃。                                        ║
// ║    3. 生命周期 — onEnter() 在场景被推入栈顶时调用，         ║
// ║       onExit() 在场景被移除/覆盖时调用。                   ║
// ║       enter → (update/render 循环) → exit                 ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    - SceneManager：持有 IScene 指针并管理其生命周期          ║
// ║    - Application：通过 SceneManager 间接与所有场景交互       ║
// ║    - 所有具体场景（TitleScene, GameplayScene 等）继承此类    ║
// ║                                                          ║
// ║  【用法】继承 IScene，实现 update() 和 render()，           ║
// ║    需要切换场景时调用 requestReplace(new OtherScene())。    ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝
class IScene
{
public:
    virtual ~IScene() = default;

    // 场景被激活时调用一次（推入栈顶或覆盖上层场景后重新暴露）
    virtual void onEnter() {}

    // 场景被停用前调用一次（被覆盖或弹出前）
    virtual void onExit() {}

    // 窗口事件（键盘、鼠标等），由 Application 分发
    virtual void handleEvent(const sf::Event& event) {}

    // 每帧逻辑更新，dt = 距上一帧秒数
    virtual void update(float dt) = 0;

    // 每帧渲染（RenderTarget = RenderWindow 或 RenderTexture）
    virtual void render(sf::RenderTarget& target) = 0;

    // ── 场景切换请求（延迟到帧末执行） ──
    void requestPush(std::unique_ptr<IScene> scene);
    void requestPop();
    void requestReplace(std::unique_ptr<IScene> scene);

    // SceneManager 在场景入栈时注入此指针
    void setSceneManager(SceneManager* manager) { m_sceneManager = manager; }
    SceneManager* getSceneManager() const { return m_sceneManager; }

protected:
    SceneManager* m_sceneManager = nullptr;
};
