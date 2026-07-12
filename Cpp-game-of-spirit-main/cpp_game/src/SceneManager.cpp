#include "SceneManager.h"
#include "IScene.h"

void SceneManager::pushScene(std::unique_ptr<IScene> scene) {
    scene->setSceneManager(this);
    scene->onEnter();
    m_scenes.push_back(std::move(scene));
}

void SceneManager::popScene() {
    if (m_scenes.empty()) return;
    m_scenes.back()->onExit();
    m_scenes.pop_back();
    if (!m_scenes.empty()) {
        m_scenes.back()->onEnter();
    }
}

void SceneManager::replaceScene(std::unique_ptr<IScene> scene) {
    if (m_scenes.empty()) {
        pushScene(std::move(scene));
        return;
    }
    scene->setSceneManager(this);
    m_scenes.back()->onExit();
    m_scenes.back() = std::move(scene);
    m_scenes.back()->onEnter();
}

void SceneManager::handleEvent(const sf::Event& event) {
    if (!m_scenes.empty()) {
        m_scenes.back()->handleEvent(event);
    }
}

void SceneManager::update(float dt) {
    if (!m_scenes.empty()) {
        m_scenes.back()->update(dt);
        applyPendingChanges();
    }
}

void SceneManager::render(sf::RenderTarget& target) {
    if (!m_scenes.empty()) {
        m_scenes.back()->render(target);
    }
}

void SceneManager::applyPendingChanges() {
    switch (m_pending.action) {
    case Action::Push:
        pushScene(std::move(m_pending.scene));
        break;
    case Action::Pop:
        popScene();
        break;
    case Action::Replace:
        replaceScene(std::move(m_pending.scene));
        break;
    default:
        break;
    }
    m_pending.action = Action::None;
    m_pending.scene.reset();
}
