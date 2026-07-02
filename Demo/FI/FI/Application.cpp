#include "Application.h"

Application::Application(const std::string& title, unsigned int width, unsigned int height)
{
    sf::VideoMode mode({ width, height });
    m_window.create(mode, title, sf::Style::Default);
    m_window.setFramerateLimit(60);
    m_window.setVerticalSyncEnabled(true);
    m_window.setKeyRepeatEnabled(false);
}

void Application::run()
{
    sf::Clock fpsClock;
    int frameCount = 0;

    while (m_window.isOpen() && !m_sceneManager.isEmpty())
    {
        m_deltaTime = m_clock.restart().asSeconds();

        processEvents();
        update();
        render();

        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f)
        {
            frameCount = 0;
            fpsClock.restart();
        }
    }
}

void Application::processEvents()
{
    while (const auto event = m_window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            m_window.close();
            return;
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>())
        {
            sf::View view(sf::FloatRect(
                { 0.0f, 0.0f },
                { static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) }
            ));
            m_window.setView(view);
        }

        m_sceneManager.handleEvent(*event);
    }
}

void Application::update()
{
    m_sceneManager.update(m_deltaTime);
}

void Application::render()
{
    m_window.clear(sf::Color::Black);
    m_sceneManager.render(m_window);
    m_window.display();
}
