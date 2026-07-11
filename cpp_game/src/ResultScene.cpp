#include "ResultScene.h"
#include "GameplayScene.h"
#include "SceneManager.h"
#include <SFML/Graphics.hpp>
#include <memory>

ResultScene::ResultScene(const ResultData& data, const std::string& grade)
    : m_data(data), m_grade(grade)
{
    if (!m_font.openFromFile("assets/fonts/msyh.ttf")) return;
    m_fontLoaded = true;

    m_titleText.emplace(m_font, "Result", 48);
    m_titleText->setFillColor(sf::Color::White);
    auto tb = m_titleText->getLocalBounds();
    m_titleText->setOrigin({tb.size.x / 2.0f, 0.0f});
    m_titleText->setPosition({640.0f, 50.0f});

    m_scoreText.emplace(m_font, "Score: " + std::to_string(data.score), 36);
    m_scoreText->setFillColor(sf::Color(255, 200, 100));
    auto sb = m_scoreText->getLocalBounds();
    m_scoreText->setOrigin({sb.size.x / 2.0f, 0.0f});
    m_scoreText->setPosition({640.0f, 130.0f});

    m_gradeText.emplace(m_font, "Grade: " + grade, 48);
    m_gradeText->setFillColor(grade == "S" ? sf::Color::Yellow : sf::Color::White);
    auto gb = m_gradeText->getLocalBounds();
    m_gradeText->setOrigin({gb.size.x / 2.0f, 0.0f});
    m_gradeText->setPosition({640.0f, 200.0f});

    m_perfectText.emplace(m_font, "Perfect: " + std::to_string(data.perfectCount), 24);
    m_perfectText->setFillColor(sf::Color::Yellow);
    m_perfectText->setPosition({540.0f, 290.0f});

    m_greatText.emplace(m_font, "Great:   " + std::to_string(data.greatCount), 24);
    m_greatText->setFillColor(sf::Color::Cyan);
    m_greatText->setPosition({540.0f, 325.0f});

    m_goodText.emplace(m_font, "Good:    " + std::to_string(data.goodCount), 24);
    m_goodText->setFillColor(sf::Color::Green);
    m_goodText->setPosition({540.0f, 360.0f});

    m_missText.emplace(m_font, "Miss:    " + std::to_string(data.missCount), 24);
    m_missText->setFillColor(sf::Color::Red);
    m_missText->setPosition({540.0f, 395.0f});

    m_comboText.emplace(m_font, "Max Combo: " + std::to_string(data.maxCombo), 30);
    m_comboText->setFillColor(sf::Color(0, 255, 200));
    auto cb = m_comboText->getLocalBounds();
    m_comboText->setOrigin({cb.size.x / 2.0f, 0.0f});
    m_comboText->setPosition({640.0f, 460.0f});

    // 节奏大师：Retry / Continue 按钮
    m_retryText.emplace(m_font, "Retry", 32);
    m_retryText->setFillColor(sf::Color::Yellow);
    auto rt = m_retryText->getLocalBounds();
    m_retryText->setOrigin({rt.size.x / 2.0f, 0.0f});
    m_retryText->setPosition({480.0f, 560.0f});

    m_continueText.emplace(m_font, "Song List", 32);
    m_continueText->setFillColor(sf::Color(200, 200, 200));
    auto ct = m_continueText->getLocalBounds();
    m_continueText->setOrigin({ct.size.x / 2.0f, 0.0f});
    m_continueText->setPosition({800.0f, 560.0f});
}

void ResultScene::onEnter() { m_timer = 0.0f; }

void ResultScene::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scan::R || key->code == sf::Keyboard::Key::R)
            requestReplace(std::make_unique<GameplayScene>());
        else
            requestPop();
        return;
    }
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(mouseBtn->position);
            if (m_retryText.has_value() && m_retryText->getGlobalBounds().contains(mousePos))
                requestReplace(std::make_unique<GameplayScene>());
            else if (m_continueText.has_value() && m_continueText->getGlobalBounds().contains(mousePos))
                requestPop();
            else
                requestPop();  // click anywhere else = continue
            return;
        }
    }
}

void ResultScene::update(float dt) { m_timer += dt; }

void ResultScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(10, 5, 20));
    if (!m_fontLoaded) return;
    if (m_titleText.has_value()) target.draw(*m_titleText);
    if (m_scoreText.has_value()) target.draw(*m_scoreText);
    if (m_gradeText.has_value()) target.draw(*m_gradeText);
    if (m_perfectText.has_value()) target.draw(*m_perfectText);
    if (m_greatText.has_value()) target.draw(*m_greatText);
    if (m_goodText.has_value()) target.draw(*m_goodText);
    if (m_missText.has_value()) target.draw(*m_missText);
    if (m_comboText.has_value()) target.draw(*m_comboText);
    if (m_retryText.has_value()) target.draw(*m_retryText);
    if (m_continueText.has_value()) target.draw(*m_continueText);
}
