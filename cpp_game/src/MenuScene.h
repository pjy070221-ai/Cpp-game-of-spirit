#pragma once

#include "IScene.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>

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

    sf::Font m_font;
    bool m_fontLoaded = false;
    std::vector<std::string> m_items;
    int m_selection = 0;
    std::optional<sf::Text> m_titleText;
    std::vector<sf::Text> m_itemTexts;
    float m_animTimer = 0.0f;
};
