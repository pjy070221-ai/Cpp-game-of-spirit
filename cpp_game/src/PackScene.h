#pragma once

#include "IScene.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>

class PackScene : public IScene {
public:
    PackScene();
    ~PackScene() override = default;

    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    void activateItem(int index);
    void updateSelectionVisuals();
    void rebuildItemTexts();

    const sf::Font* m_font = nullptr;
    bool m_fontLoaded = false;
    std::vector<sf::String> m_songNames;
    std::vector<std::string> m_chartBases;
    int m_selection = 0;
    std::optional<sf::Text> m_titleText;
    std::vector<sf::Text> m_itemTexts;
};

