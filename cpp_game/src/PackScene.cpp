#include "PackScene.h"
#include "GameplayScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "MenuScene.h"
#include <SFML/Graphics.hpp>
#include <memory>

PackScene::PackScene()
    : m_songNames({sf::String(L"Beat Demo"), sf::String(L"\u6E38\u4EAC"),
                sf::String(L"Infinite Strife"), sf::String(L"Pentiment"),
                sf::String(L"Practice Mode")})
    , m_chartPaths({"song_demo.json", "song.json", "song_infinite_strife.json", "song_pentiment.json", ""})
{
    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (!fontPtr) return;
    m_font = *fontPtr;
    m_fontLoaded = true;

    m_titleText.emplace(m_font, "Select Song", 48);
    m_titleText->setFillColor(sf::Color::White);
    auto tb = m_titleText->getLocalBounds();
    m_titleText->setOrigin({tb.size.x / 2.0f, 0.0f});
    m_titleText->setPosition({640.0f, 100.0f});

    for (int i = 0; i < (int)m_songNames.size(); ++i) {
        sf::Text txt(m_font, m_songNames[i], 36);
        txt.setFillColor(i == 0 ? sf::Color::Yellow : sf::Color(200, 200, 200));
        auto ib = txt.getLocalBounds();
        txt.setOrigin({ib.size.x / 2.0f, 0.0f});
        txt.setPosition({640.0f, 250.0f + i * 100.0f});
        m_itemTexts.push_back(txt);
    }
}

void PackScene::onEnter() {
    m_selection = 0;
    updateSelectionVisuals();
}

void PackScene::handleEvent(const sf::Event& event) {
    // 闂佸啿鍘滈崑鎾绘煃閸忓�?闂備焦顑欓崰姘鸿箛鏇楀亾娴ｅ啫顥嶉�?闂佸啿鍘滈崑鎾绘煃閸忓�?
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        bool changed = false;
        if (key->scancode == sf::Keyboard::Scan::Up || key->code == sf::Keyboard::Key::Up) {
            m_selection = (m_selection - 1 + (int)m_songNames.size()) % m_songNames.size();
            changed = true;
        } else if (key->scancode == sf::Keyboard::Scan::Down || key->code == sf::Keyboard::Key::Down) {
            m_selection = (m_selection + 1) % m_songNames.size();
            changed = true;
        } else if (key->scancode == sf::Keyboard::Scan::Enter || key->code == sf::Keyboard::Key::Enter) {
            activateItem(m_selection);
            return;
        } else if (key->scancode == sf::Keyboard::Scan::Escape || key->code == sf::Keyboard::Key::Escape) {
            requestPop();
            requestReplace(std::make_unique<MenuScene>());
            return;
        }
        if (changed) updateSelectionVisuals();
        return;
    }

    // 闂佸啿鍘滈崑鎾绘煃閸忓�?婵崿鍛ｉ柣鏍电秮楠炲啴顢楅埀顒佺閻樻剚娈楁俊顖滄嚀�?闂佸啿鍘滈崑鎾绘煃閸忓�?
    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(mouseMoved->position);
        for (int i = 0; i < (int)m_itemTexts.size(); ++i) {
            if (m_itemTexts[i].getGlobalBounds().contains(mousePos)) {
                if (m_selection != i) {
                    m_selection = i;
                    updateSelectionVisuals();
                }
                return;
            }
        }
        return;
    }

    // 闂佸啿鍘滈崑鎾绘煃閸忓�?婵崿鍛ｉ柣鏍电秮閹瑩鎮烽弶鎸庣�?闂佸啿鍘滈崑鎾绘煃閸忓�?
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(mouseBtn->position);
            for (int i = 0; i < (int)m_itemTexts.size(); ++i) {
                if (m_itemTexts[i].getGlobalBounds().contains(mousePos)) {
                    activateItem(i);
                    return;
                }
            }
        }
        return;
    }
}

void PackScene::activateItem(int index) {
    GameplayScene::s_chartPath = m_chartPaths[index];
    requestPush(std::make_unique<GameplayScene>());
}

void PackScene::updateSelectionVisuals() {
    for (int i = 0; i < (int)m_itemTexts.size(); ++i)
        m_itemTexts[i].setFillColor(i == m_selection ? sf::Color::Yellow : sf::Color(200, 200, 200));
}

void PackScene::update(float dt) {
    if (SceneManager::s_transitionAlpha > 0.001f)
        SceneManager::s_transitionAlpha = std::max(0.0f, SceneManager::s_transitionAlpha - dt * 2.5f);
}

void PackScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(10, 5, 20));
    if (!m_fontLoaded) return;
    if (m_titleText.has_value()) target.draw(*m_titleText);
    for (auto& t : m_itemTexts) target.draw(t);

    // 闂佸吋瀵х划灞界暦閻斿憡浜ら柛銉ｅ妽鐠囩偤姊洪澶婃灓闁稿秹娼ч妴鎺楀箛椤掆偓�?
    if (SceneManager::s_transitionAlpha > 0.001f) {
        sf::RectangleShape ov({1280, 720});
        ov.setFillColor(sf::Color(0, 0, 0, (std::uint8_t)(SceneManager::s_transitionAlpha * 255)));
        target.draw(ov);
    }
}






