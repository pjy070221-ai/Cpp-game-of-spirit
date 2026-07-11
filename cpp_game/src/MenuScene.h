#pragma once

#include "IScene.h"
#include "SceneManager.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>
#include <cmath>
#include <random>

class MenuScene : public IScene {
public:
    MenuScene();
    ~MenuScene() override = default;
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
private:
    void activateItem(int index);
    void updateSelectionVisuals();
    void startTransition(int targetIndex);
    sf::Color getNeonColor(float time) const;

    sf::Font m_font;
    bool m_fontLoaded = false;
    std::vector<std::string> m_items;
    int m_selection = 0;
    std::optional<sf::Text> m_titleText;
    std::vector<sf::Text> m_itemTexts;
    float m_animTimer = 0.0f;

    // 浮动光效
    struct FloatLight {
        sf::CircleShape shape;
        float angle, orbitRadiusX, orbitRadiusY, speed;
    };
    std::vector<FloatLight> m_floatLights;
    float m_titleCenterX = 640.0f, m_titleCenterY = 150.0f;

    // 选项发光
    sf::RectangleShape m_optionGlow, m_optionGlow2;

    // 过渡
    bool m_transitioning = false;
    float m_fadeTimer = 0.0f, m_flashTimer = 0.0f;
    int m_pendingIndex = -1;
    sf::RectangleShape m_fadeOverlay, m_flashOverlay;
};
