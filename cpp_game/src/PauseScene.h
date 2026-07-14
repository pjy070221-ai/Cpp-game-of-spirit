#pragma once

#include "IScene.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>

class PauseScene : public IScene {
public:
    PauseScene();
    ~PauseScene() override = default;

    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    void activateItem(int index);
    void updateSelectionVisuals();

    const sf::Font* m_font = nullptr;
    bool m_fontLoaded = false;
    int m_selection = 0;
    std::optional<sf::Text> m_titleText;
    std::vector<sf::Text> m_itemTexts;
};


