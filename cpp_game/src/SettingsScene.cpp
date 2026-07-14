#include "SettingsScene.h"
#include "SettingsData.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <memory>

SettingsScene::SettingsScene()
    : m_labels({"Volume", "Fullscreen", "Auto Play", "Difficulty"})
{
    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (!fontPtr) return;
    m_font = *fontPtr;
    m_fontLoaded = true;

    SettingsData s;
    m_values = {s.getMasterVolume(),
                s.getFullscreen() ? 1.0f : 0.0f,
                s.getAutoPlay() ? 1.0f : 0.0f,
                (float)s.getDifficulty()};

    m_titleText.emplace(m_font, "Settings", 48);
    m_titleText->setFillColor(sf::Color::White);
    auto tb = m_titleText->getLocalBounds();
    m_titleText->setOrigin({tb.size.x / 2.0f, 0.0f});
    m_titleText->setPosition({640.0f, 50.0f});

    refreshDisplay();
}

void SettingsScene::onEnter() {
    m_selection = 0;
    updateSelectionVisuals();
}

void SettingsScene::handleEvent(const sf::Event& event) {
    // ── 键盘 ──
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scan::Up || key->code == sf::Keyboard::Key::Up) {
            m_selection = (m_selection - 1 + (int)m_labels.size()) % m_labels.size();
            updateSelectionVisuals();
            return;
        }
        if (key->scancode == sf::Keyboard::Scan::Down || key->code == sf::Keyboard::Key::Down) {
            m_selection = (m_selection + 1) % m_labels.size();
            updateSelectionVisuals();
            return;
        }
        if (key->scancode == sf::Keyboard::Scan::Escape || key->code == sf::Keyboard::Key::Escape) {
            requestPop();
            return;
        }
        // ── 修改数值 ──
        float delta = 0.0f;
        if (key->scancode == sf::Keyboard::Scan::Right || key->code == sf::Keyboard::Key::Right)
            delta = 1.0f;
        else if (key->scancode == sf::Keyboard::Scan::Left || key->code == sf::Keyboard::Key::Left)
            delta = -1.0f;
        else
            return;

        float& v = m_values[m_selection];
        SettingsData s;
        if (m_selection == 0) {           // 音量 0-1
            v = std::clamp(v + delta * 0.05f, 0.0f, 1.0f);
            s.setMasterVolume(v);
        } else if (m_selection == 1) {    // 全屏切换
            v = (v > 0.5f) ? 0.0f : 1.0f;
            s.setFullscreen(v > 0.5f);
        } else if (m_selection == 2) {  // Auto Play 开关
            v = (v > 0.5f) ? 0.0f : 1.0f;
            s.setAutoPlay(v > 0.5f);
        } else if (m_selection == 3) {  // Difficulty 切换
            v = (v > 0.5f) ? 0.0f : 1.0f;
            s.setDifficulty((int)(v + 0.1f));
        }
        refreshDisplay();
        return;
    }

    // ── 鼠标悬停 ──
    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(mouseMoved->position);
        for (int i = 0; i < (int)m_lineTexts.size(); ++i) {
            if (m_lineTexts[i].getGlobalBounds().contains(mousePos)) {
                if (m_selection != i) { m_selection = i; updateSelectionVisuals(); }
                return;
            }
        }
        return;
    }

    // ── 鼠标点击 ──
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(mouseBtn->position);
            for (int i = 0; i < (int)m_lineTexts.size(); ++i) {
                if (m_lineTexts[i].getGlobalBounds().contains(mousePos)) {
                    if (m_selection != i) { m_selection = i; updateSelectionVisuals(); return; }
                }
            }
        }
        return;
    }
}

void SettingsScene::refreshDisplay() {
    m_lineTexts.clear();
    for (int i = 0; i < (int)m_labels.size(); ++i) {
        std::string val;
        float v = m_values[i];
       if (i == 1) val = (v > 0.5f) ? "Yes" : "No";
       else if (i == 2) val = (v > 0.5f) ? "ON" : "OFF";
        else if (i == 3) val = (v > 0.5f) ? "Hard" : "Easy";
       else { std::ostringstream oss; oss << std::fixed << std::setprecision(1) << v; val = oss.str(); }

        sf::Text txt(m_font, m_labels[i] + ": " + val, 28);
        txt.setFillColor(i == m_selection ? sf::Color::Yellow : sf::Color(200, 200, 200));
        auto tb = txt.getLocalBounds();
        txt.setOrigin({tb.size.x / 2.0f, 0.0f});
        txt.setPosition({640.0f, 150.0f + i * 80.0f});
        m_lineTexts.push_back(txt);
    }
}

void SettingsScene::updateSelectionVisuals() {
    for (int i = 0; i < (int)m_lineTexts.size(); ++i)
        m_lineTexts[i].setFillColor(i == m_selection ? sf::Color::Yellow : sf::Color(200, 200, 200));
}

void SettingsScene::update(float) {}

void SettingsScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(10, 5, 20));
    if (!m_fontLoaded) return;
    if (m_titleText.has_value()) target.draw(*m_titleText);
    for (auto& t : m_lineTexts) target.draw(t);
}
