#pragma once
#include "IScene.h"
#include "../core/ParticleSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class PackScene : public IScene
{
public:
    PackScene();
    ~PackScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    // ═══════════════════════════════════════════════
    // 歌曲数据
    // ═══════════════════════════════════════════════
    struct SongInfo {
        std::string title;
        int difficulty = 1;
    };
    struct PackData {
        std::string name;
        std::string sub;
        sf::Color themeColor;
        std::vector<SongInfo> songs;
    };
    std::vector<PackData> m_packs;

    // ═══════════════════════════════════════════════
    // 导航状态
    // ═══════════════════════════════════════════════
    enum class NavLevel { Pack, Song };
    NavLevel m_navLevel = NavLevel::Pack;
    int m_selectedPack = 0;
    int m_selectedSong = 0;

    // ═══════════════════════════════════════════════
    // 卡片视觉
    // ═══════════════════════════════════════════════
    struct CardVisual {
        sf::RectangleShape bg;
        sf::RectangleShape border;
        sf::Text nameText;
        sf::Text subText;
        CardVisual(const sf::Font& font) : nameText(font), subText(font) {}
        float currentX = 0.f, currentY = 0.f, currentScale = 1.f;
        float targetX = 0.f, targetY = 0.f, targetScale = 1.f;
    };
    std::vector<CardVisual> m_cards;
    static constexpr float CARD_W = 260.f, CARD_H = 340.f;
    static constexpr float CARD_SPACING = 310.f;
    static constexpr float CENTER_X = 960.f, CENTER_Y = 420.f;
    static constexpr float SIDE_Y = 455.f, SIDE_SCALE = 0.72f;

    // ═══════════════════════════════════════════════
    // 歌曲列表渲染
    // ═══════════════════════════════════════════════
    std::vector<sf::Text> m_songTexts;
    sf::RectangleShape m_songListBg;
    sf::Text m_songTitle;

    // ═══════════════════════════════════════════════
    // 顶部栏（角色 + 潜力值）
    // ═══════════════════════════════════════════════
    sf::Text m_charLabel;
    sf::Text m_potentialLabel;

    // ═══════════════════════════════════════════════
    // 动画 & 通用
    // ═══════════════════════════════════════════════
    float m_elapsedTime = 0.f;
    float m_glowPulse = 0.f;
    sf::Font m_font;
    sf::Text m_titleText;
    sf::Text m_hintText;
    sf::Text m_pageDots;
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    ParticleSystem m_stars;

    // ═══════════════════════════════════════════════
    // 方法
    // ═══════════════════════════════════════════════
    void buildBackground();
    void buildPacks();
    void buildCards();
    void buildSongList();
    void buildTopBar();
    void updateCardTargets();
    void updateCards(float dt);
    void updateSongTexts();
    std::string hintText() const;
};
