#include "IScene.h"
#include "SceneManager.h"

void IScene::requestPush(std::unique_ptr<IScene> scene) {
    m_sceneManager->m_pending = {SceneManager::Action::Push, std::move(scene)};
}

void IScene::requestPop() {
    m_sceneManager->m_pending = {SceneManager::Action::Pop, nullptr};
}

void IScene::requestReplace(std::unique_ptr<IScene> scene) {
    m_sceneManager->m_pending = {SceneManager::Action::Replace, std::move(scene)};
}

