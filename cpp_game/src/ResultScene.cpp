#include "ResultScene.h"
#include "GameplayScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Easing.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <random>
#include <algorithm>

ResultScene::ResultScene(const ResultData& data, const std::string& grade)
    : m_data(data), m_grade(grade)
{
    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (!fontPtr) return;
    m_font = fontPtr;
    m_fontLoaded = true;

    m_titleText.emplace(*m_font, L"\u7ED3\u7B97", 48);
    m_titleText->setFillColor(sf::Color::White);
    auto tb = m_titleText->getLocalBounds();
    m_titleText->setOrigin({tb.size.x / 2.0f, 0.0f});
    m_titleText->setPosition({640.0f, 50.0f});

    m_scoreText.emplace(*m_font, sf::String(L"\u5206\u6570: ") + std::to_string(data.score), 36);
    m_scoreText->setFillColor(sf::Color(255, 200, 100));
    auto sb = m_scoreText->getLocalBounds();
    m_scoreText->setOrigin({sb.size.x / 2.0f, 0.0f});
    m_scoreText->setPosition({640.0f, 130.0f});

    auto gc = gradeColor();
    m_gradeText.emplace(*m_font, sf::String(L"\u8BC4\u7EA7: ") + grade, 48);
    m_gradeText->setFillColor(gc);
    auto gb = m_gradeText->getLocalBounds();
    m_gradeText->setOrigin({gb.size.x / 2.0f, 0.0f});
    m_gradeText->setPosition({640.0f, 200.0f});

    sf::String cele;
    if (grade == "S") cele = L"\u5B8C\u7F8E\u6F14\u594F!";
    else if (grade == "A") cele = L"\u592A\u68D2\u4E86!";
    else if (grade == "B") cele = L"\u8868\u73B0\u4E0D\u9519!";
    else if (grade == "C") cele = L"\u7EE7\u7EED\u52A0\u6CB9!";
    else cele = L"\u518D\u8BD5\u4E00\u6B21!";
    m_celebrationText.emplace(*m_font, cele, 28);
    m_celebrationText->setFillColor(gc);
    m_celebrationText->setPosition({640, 260});
    m_celebrationText->setOrigin({m_celebrationText->getLocalBounds().size.x / 2.0f, 0});
    m_celebrationText->setScale({2.0f, 2.0f});
    m_celebrationText->setFillColor(sf::Color(gc.r, gc.g, gc.b, 0));

    m_perfectText.emplace(*m_font, sf::String(L"Perfect: ") + std::to_string(data.perfectCount), 24);
    m_perfectText->setFillColor(sf::Color::Yellow);
    m_perfectText->setPosition({540.0f, 290.0f});

    m_greatText.emplace(*m_font, sf::String(L"Great:   ") + std::to_string(data.greatCount), 24);
    m_greatText->setFillColor(sf::Color::Cyan);
    m_greatText->setPosition({540.0f, 325.0f});

    m_goodText.emplace(*m_font, sf::String(L"Good:    ") + std::to_string(data.goodCount), 24);
    m_goodText->setFillColor(sf::Color::Green);
    m_goodText->setPosition({540.0f, 360.0f});

    m_missText.emplace(*m_font, sf::String(L"Miss:    ") + std::to_string(data.missCount), 24);
    m_missText->setFillColor(sf::Color::Red);
    m_missText->setPosition({540.0f, 395.0f});

    m_comboText.emplace(*m_font, sf::String(L"\u6700\u9AD8Combo: ") + std::to_string(data.maxCombo), 30);
    m_comboText->setFillColor(sf::Color(0, 255, 200));
    auto cb = m_comboText->getLocalBounds();
    m_comboText->setOrigin({cb.size.x / 2.0f, 0.0f});
    m_comboText->setPosition({640.0f, 460.0f});

    m_retryText.emplace(*m_font, L"\u91CD\u8BD5", 32);
    m_retryText->setFillColor(sf::Color::Yellow);
    auto rt = m_retryText->getLocalBounds();
    m_retryText->setOrigin({rt.size.x / 2.0f, 0.0f});
    m_retryText->setPosition({480.0f, 560.0f});

    m_continueText.emplace(*m_font, L"\u6B4C\u66F2\u5217\u8868", 32);
    m_continueText->setFillColor(sf::Color(200, 200, 200));
    auto ct = m_continueText->getLocalBounds();
    m_continueText->setOrigin({ct.size.x / 2.0f, 0.0f});
    m_continueText->setPosition({800.0f, 560.0f});
}

void ResultScene::onEnter() { m_timer = 0.0f; m_lastBurst = 0; m_fireworks.clear(); }

sf::Color ResultScene::gradeColor() const {
    if (m_grade == "S") return sf::Color(255, 215, 0);
    if (m_grade == "A") return sf::Color(0, 255, 255);
    if (m_grade == "B") return sf::Color(100, 255, 100);
    if (m_grade == "C") return sf::Color(100, 200, 255);
    return sf::Color(200, 200, 200);
}

void ResultScene::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scan::R || key->code == sf::Keyboard::Key::R)
            requestReplace(std::make_unique<GameplayScene>());
        else if (m_timer > 1.8f)
            requestPop();
        return;
    }
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(mouseBtn->position);
            if (m_timer > 1.8f) {
                if (m_retryText.has_value() && m_retryText->getGlobalBounds().contains(mousePos))
                    requestReplace(std::make_unique<GameplayScene>());
                else
                    requestPop();
            }
            return;
        }
    }
}

void ResultScene::update(float dt) {
    m_timer += dt;
    float t = m_timer;

    if (m_titleText.has_value()) {
        float p = std::clamp((t - 0.0f) / 0.5f, 0.0f, 1.0f);
        m_titleText->setPosition({640, 50 - 80 * (1 - easeOutCubic(p))});
    }

    if (m_gradeText.has_value()) {
        float p = std::clamp((t - 0.2f) / 0.6f, 0.0f, 1.0f);
        float e = easeOutBack(p);
        m_gradeText->setPosition({640, 200 - 300 * (1 - e)});
    }

    if (m_celebrationText.has_value()) {
        float p = std::clamp((t - 0.5f) / 0.5f, 0.0f, 1.0f);
        float e = easeOutBack(p);
        float s = 2.0f - e;
        auto c = gradeColor();
        c.a = (std::uint8_t)(e * 255);
        m_celebrationText->setScale({s, s});
        m_celebrationText->setFillColor(c);
    }

    if (m_scoreText.has_value()) {
        float p = std::clamp((t - 0.7f) / 0.4f, 0.0f, 1.0f);
        m_scoreText->setPosition({900 - 260 * easeOutCubic(p), 130});
    }

    // Stats: use pointer array for iteration
    std::optional<sf::Text>* statPtrs[4] = {&m_perfectText, &m_greatText, &m_goodText, &m_missText};
    float dlys[4] = {1.0f, 1.1f, 1.2f, 1.3f};
    for (int si = 0; si < 4; si++) {
        if (statPtrs[si]->has_value()) {
            float p = std::clamp((t - dlys[si]) / 0.3f, 0.0f, 1.0f);
            (**statPtrs[si]).setPosition({540 - 200 * (1 - easeOutBack(p)), 290.0f + si * 35.0f});
        }
    }

    if (m_comboText.has_value()) {
        float p = std::clamp((t - 1.5f) / 0.5f, 0.0f, 1.0f);
        m_comboText->setPosition({640, 460 - 200 * (1 - easeOutBack(p))});
    }

    float btnA = std::clamp((t - 1.9f) / 0.3f, 0.0f, 1.0f);
    auto setBtnA = [&](std::optional<sf::Text>& tx, sf::Color base) {
        if (tx.has_value()) { base.a = (std::uint8_t)(btnA * 255); tx->setFillColor(base); }
    };
    setBtnA(m_retryText, sf::Color::Yellow);
    setBtnA(m_continueText, sf::Color(200, 200, 200));

    // Fireworks
    auto fc = gradeColor();
    auto burst = [&](float trg, sf::Color col, int cnt) {
        if (t >= trg && m_lastBurst < (int)(trg * 10 + 1)) {
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<float> xd(100, 1180), yd(50, 350);
            for (int i = 0; i < 3; i++)
                m_fireworks.emit({xd(rng), yd(rng)}, cnt / 3, col, 80, 300, 0.5f, 1.5f, 2, 8);
        }
    };
    burst(1.0f, fc, 60); burst(1.8f, sf::Color::White, 40);
    burst(2.5f, fc, 80); burst(3.5f, sf::Color(255, 200, 100), 50);
    m_lastBurst = (int)(t * 10);

    m_fireworks.update(dt);
}

void ResultScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(10, 5, 20));
    if (!m_fontLoaded) return;
    m_fireworks.render(target);
    if (m_titleText.has_value()) target.draw(*m_titleText);
    if (m_gradeText.has_value()) target.draw(*m_gradeText);
    if (m_celebrationText.has_value()) target.draw(*m_celebrationText);
    if (m_scoreText.has_value()) target.draw(*m_scoreText);
    if (m_perfectText.has_value()) target.draw(*m_perfectText);
    if (m_greatText.has_value()) target.draw(*m_greatText);
    if (m_goodText.has_value()) target.draw(*m_goodText);
    if (m_missText.has_value()) target.draw(*m_missText);
    if (m_comboText.has_value()) target.draw(*m_comboText);
    if (m_retryText.has_value()) target.draw(*m_retryText);
    if (m_continueText.has_value()) target.draw(*m_continueText);
}
