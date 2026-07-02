#include "SceneManager.h"

// ═══════════════════════════════════════════════
// IScene 的请求方法 — 委托给 SceneManager
// ═══════════════════════════════════════════════
void IScene::requestPush(std::unique_ptr<IScene> scene)
{
    if (m_sceneManager)
        m_sceneManager->pushScene(std::move(scene));
}

void IScene::requestPop()
{
    if (m_sceneManager)
        m_sceneManager->popScene();
}

void IScene::requestReplace(std::unique_ptr<IScene> scene)
{
    if (m_sceneManager)
        m_sceneManager->replaceScene(std::move(scene));
}

// ═══════════════════════════════════════════════
// SceneManager 实现
// ═══════════════════════════════════════════════

void SceneManager::pushScene(std::unique_ptr<IScene> scene)
{
    // 如果栈为空（如 main() 中首次调用），立即应用，避免主循环因 isEmpty() 直接退出
    if (m_scenes.empty())
    {
        m_scenes.push_back(std::move(scene));
        m_scenes.back()->setSceneManager(this);
        m_scenes.back()->onEnter();
        return;
    }
    m_pending.action = Action::Push;
    m_pending.scene = std::move(scene);
}

void SceneManager::popScene()
{
    m_pending.action = Action::Pop;
    m_pending.scene = nullptr;
}

void SceneManager::replaceScene(std::unique_ptr<IScene> scene)
{
    m_pending.action = Action::Replace;
    m_pending.scene = std::move(scene);
}

void SceneManager::applyPendingChanges()
{
    switch (m_pending.action)
    {
    case Action::Push:
        // 当前场景先 onExit
        if (!m_scenes.empty())
            m_scenes.back()->onExit();
        m_scenes.push_back(std::move(m_pending.scene));
        m_scenes.back()->setSceneManager(this);
        m_scenes.back()->onEnter();
        break;

    case Action::Pop:
        if (!m_scenes.empty())
        {
            m_scenes.back()->onExit();
            m_scenes.pop_back();
        }
        if (!m_scenes.empty())
            m_scenes.back()->onEnter();
        break;

    case Action::Replace:
        if (!m_scenes.empty())
        {
            m_scenes.back()->onExit();
            m_scenes.pop_back();
        }
        m_scenes.push_back(std::move(m_pending.scene));
        m_scenes.back()->setSceneManager(this);
        m_scenes.back()->onEnter();
        break;

    default:
        break;
    }

    m_pending.action = Action::None;
    m_pending.scene = nullptr;
}

void SceneManager::handleEvent(const sf::Event& event)
{
    if (!m_scenes.empty())
        m_scenes.back()->handleEvent(event);
}

void SceneManager::update(float dt)
{
    if (!m_scenes.empty())
        m_scenes.back()->update(dt);

    // 帧末执行挂起的场景切换
    applyPendingChanges();
}

void SceneManager::render(sf::RenderTarget& target)
{
    for (auto& scene : m_scenes)
        scene->render(target);
}
