#pragma once

#include <vector>
#include <memory>
#include "scenes/IScene.h"

// ╔══════════════════════════════════════════════════════════╗
// ║  SceneManager — 场景栈管理器                               ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】管理场景的生命周期与切换。用 vector 模拟栈结构：    ║
// ║    - pushScene()   : 在栈顶新增场景（旧场景保留在底下）    ║
// ║    - popScene()    : 移除栈顶场景（回到上一层）            ║
// ║    - replaceScene(): 替换栈顶场景（旧场景销毁）            ║
// ║                                                          ║
// ║  【延迟切换机制 — 核心安全设计】                            ║
// ║    场景切换请求不是立即执行的。当场景 A 的 update() 中调用   ║
// ║    requestReplace(B)，实际切换发生在 update() 返回之后。    ║
// ║    这避免了"在 A 的成员函数中 delete A"的典型 C++ 灾难。     ║
// ║                                                          ║
// ║    特例：栈为空时 pushScene() 直接执行（main() 中首次调用） ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    - IScene：持有所有场景实例，分发事件/更新/渲染            ║
// ║    - Application：每帧调用 handleEvent/update/render       ║
// ║    - 每个 IScene 持有 SceneManager* 用于请求切换            ║
// ║                                                          ║
// ║  【渲染策略】从栈底到栈顶依次 render()，                   ║
// ║    栈底场景作为背景可见（如果栈顶场景有透明区域）。          ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝
class SceneManager
{
public:
    void pushScene(std::unique_ptr<IScene> scene);
    void popScene();
    void replaceScene(std::unique_ptr<IScene> scene);

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void render(sf::RenderTarget& target);

    bool isEmpty() const { return m_scenes.empty(); }

private:
    void applyPendingChanges();

    enum class Action { None, Push, Pop, Replace };

    struct PendingChange
    {
        Action action = Action::None;
        std::unique_ptr<IScene> scene;
    };

    std::vector<std::unique_ptr<IScene>> m_scenes;
    PendingChange m_pending;
};
