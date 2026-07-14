#pragma once

#include "IScene.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>

class SettingsScene : public IScene {
public:
    SettingsScene();
    ~SettingsScene() override = default;

    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    void refreshDisplay();
    void activateItem(int index);
    void updateSelectionVisuals();

    const sf::Font* m_font = nullptr;
    bool m_fontLoaded = false;
    std::vector<std::string> m_labels;  // setting display names
    std::vector<float> m_values;        // current values (float for all)
    int m_selection = 0;
    std::optional<sf::Text> m_titleText;
    std::vector<sf::Text> m_lineTexts;
};
