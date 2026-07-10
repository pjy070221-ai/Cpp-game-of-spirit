#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

class SceneManager;

class IScene {
public:
    virtual ~IScene() = default;

    // lifecycle hooks (empty by default, override as needed)
    virtual void onEnter() {}
    virtual void onExit() {}

    // core interface
    virtual void handleEvent(const sf::Event& event) {}
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    // deferred scene-switch requests
    void requestPush(std::unique_ptr<IScene> scene);
    void requestPop();
    void requestReplace(std::unique_ptr<IScene> scene);

    void setSceneManager(SceneManager* manager) { m_sceneManager = manager; }
    SceneManager* getSceneManager() const { return m_sceneManager; }

protected:
    SceneManager* m_sceneManager = nullptr;
};
