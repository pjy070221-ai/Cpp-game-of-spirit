#pragma once

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "IScene.h"

class SceneManager {
public:
    inline static float s_transitionAlpha = 0.0f;
    inline static bool s_exitRequested = false; // set by scenes to close window
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    void pushScene(std::unique_ptr<IScene> scene);
    void popScene();
    void replaceScene(std::unique_ptr<IScene> scene);

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void render(sf::RenderTarget& target);

    bool isEmpty() const { return m_scenes.empty(); }

private:
    friend class IScene;
        void applyPendingChanges();

    enum class Action { None, Push, Pop, Replace };
    struct PendingChange {
        Action action = Action::None;
        std::unique_ptr<IScene> scene;
    };

    std::vector<std::unique_ptr<IScene>> m_scenes;
    PendingChange m_pending;
};



