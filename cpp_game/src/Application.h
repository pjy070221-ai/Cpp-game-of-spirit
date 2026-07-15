#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include "SceneManager.h"
#include "SettingsData.h"

class Application {
public:
    Application(const sf::String& title, unsigned int width, unsigned int height);
    ~Application() = default;

    void run();
    void setFullscreen(bool fullscreen);

    sf::RenderWindow& getWindow();
    SceneManager& getSceneManager();
    float getDeltaTime() const;

    static Application* s_instance;

private:
    void processEvents();
    void update();
    void render();
    void updateView();

    sf::RenderWindow m_window;
    SceneManager m_sceneManager;
    sf::Clock m_clock;
    sf::String m_windowTitle;
    unsigned int m_windowWidth;
    unsigned int m_windowHeight;
    float m_deltaTime = 0.0f;
};
