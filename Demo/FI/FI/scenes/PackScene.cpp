#include "PackScene.h"
#include "TitleScene.h"
#include "GameplayScene.h"
#include "SettingsScene.h"
#include "../core/Easing.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

// ═══════════════════════════════════════════════
// 构造 & 生命周期
// ═══════════════════════════════════════════════

PackScene::PackScene()
    : m_titleText(m_font)
    , m_hintText(m_font)
    , m_pageDots(m_font)
    , m_songTitle(m_font)
    , m_charLabel(m_font)
    , m_potentialLabel(m_font)
{
    m_font.openFromFile("C:/Windows/Fonts/msyh.ttc") ||
        m_font.openFromFile("C:/Windows/Fonts/arial.ttf");

    buildBackground();
    buildTopBar();
    buildPacks();
    buildCards();
    buildSongList();
}

void PackScene::onEnter()
{
    m_elapsedTime = 0.f;
    m_glowPulse = 0.f;
    m_navLevel = NavLevel::Pack;
    m_selectedPack = 0;
    m_selectedSong = 0;

    updateCardTargets();
    for (auto& card : m_cards)
    {
        card.currentX = card.targetX + 400.f;
        card.currentY = card.targetY;
        card.currentScale = card.targetScale;
    }

    m_stars.clear();
}

void PackScene::onExit() { m_stars.clear(); }

// ═══════════════════════════════════════════════
// 背景 & 顶部栏
// ═══════════════════════════════════════════════

void PackScene::buildBackground()
{
    constexpr float W = 1920.f, H = 1080.f;
    m_bgGradient.clear();
    m_bgGradient.append({ {0, 0},       {0, 0, 0, 255} });
    m_bgGradient.append({ {W, 0},       {0, 0, 0, 255} });
    m_bgGradient.append({ {0, H*.3f},   {4, 3, 12, 200} });
    m_bgGradient.append({ {W, H*.3f},   {4, 3, 12, 200} });
    m_bgGradient.append({ {0, H*.7f},   {4, 3, 12, 200} });
    m_bgGradient.append({ {W, H*.7f},   {4, 3, 12, 200} });
    m_bgGradient.append({ {0, H},       {0, 0, 0, 255} });
    m_bgGradient.append({ {W, H},       {0, 0, 0, 255} });
}

void PackScene::buildTopBar()
{
    m_charLabel.setString("Character: ???");
    m_charLabel.setCharacterSize(18);
    m_charLabel.setFillColor({ 140, 150, 180, 180 });
    m_charLabel.setPosition({ 40.f, 22.f });

    m_potentialLabel.setString("Potential: 12.45");
    m_potentialLabel.setCharacterSize(18);
    m_potentialLabel.setFillColor({ 140, 150, 180, 180 });
    sf::FloatRect pb = m_potentialLabel.getLocalBounds();
    m_potentialLabel.setPosition({ 1880.f - pb.size.x, 22.f });
}

// ═══════════════════════════════════════════════
// 曲包 & 歌曲数据
// ═══════════════════════════════════════════════

void PackScene::buildPacks()
{
    m_packs = {
        { "ABYSSAL",  "The Depths",     { 80, 60, 180 }, {
            { "Boss Song",      11 },
            { "Depths Below",    9 },
            { "Abyssal Call",    7 },
        }},
        { "ETHEREAL", "Celestial Light",{ 60, 140, 200 }, {
            { "Celestial",       9 },
            { "Starlight",       7 },
            { "Aurora",          5 },
        }},
        { "FRACTURE", "Shattered Void", { 180, 60, 120 }, {
            { "Shatter",        13 },
            { "Crack",          11 },
            { "Rupture",         9 },
        }},
    };
}

void PackScene::buildCards()
{
    m_cards.clear();
    m_titleText.setString("SELECT PACK");
    m_titleText.setCharacterSize(28);
    m_titleText.setFillColor({ 120, 140, 180, 180 });
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin({ tb.size.x * .5f, tb.size.y * .5f });
    m_titleText.setPosition({ 960.f, 100.f });

    m_pageDots.setCharacterSize(22);
    m_pageDots.setFillColor({ 140, 160, 200, 180 });

    for (auto& pack : m_packs)
    {
        CardVisual cv(m_font);
        cv.bg.setSize({ CARD_W, CARD_H });
        cv.bg.setOrigin({ CARD_W*.5f, CARD_H*.5f });
        cv.bg.setFillColor({ 15, 12, 30, 230 });
        cv.bg.setOutlineThickness(2.f);
        cv.bg.setOutlineColor({ 40, 35, 70, 200 });

        cv.border.setSize({ CARD_W + 6.f, CARD_H + 6.f });
        cv.border.setOrigin({ (CARD_W + 6.f)*.5f, (CARD_H + 6.f)*.5f });
        cv.border.setFillColor(sf::Color::Transparent);
        cv.border.setOutlineThickness(2.f);
        cv.border.setOutlineColor({ 60, 50, 100, 180 });

        cv.nameText.setString(pack.name);
        cv.nameText.setCharacterSize(32);
        cv.nameText.setFillColor({ 220, 225, 255, 255 });
        sf::FloatRect nb = cv.nameText.getLocalBounds();
        cv.nameText.setOrigin({ nb.size.x*.5f, nb.size.y*.5f });

        cv.subText.setString(pack.sub);
        cv.subText.setCharacterSize(16);
        cv.subText.setFillColor({ 130, 150, 200, 200 });
        sf::FloatRect sb = cv.subText.getLocalBounds();
        cv.subText.setOrigin({ sb.size.x*.5f, sb.size.y*.5f });

        m_cards.push_back(std::move(cv));
    }

    updateCardTargets();
    for (auto& card : m_cards)
    {
        card.currentX = card.targetX;
        card.currentY = card.targetY;
        card.currentScale = card.targetScale;
    }
}

void PackScene::buildSongList()
{
    m_songListBg.setSize({ 500.f, 180.f });
    m_songListBg.setFillColor({ 12, 10, 28, 220 });
    m_songListBg.setOutlineThickness(1.5f);
    m_songListBg.setOutlineColor({ 60, 55, 100, 180 });

    m_songTitle.setString("SONGS");
    m_songTitle.setCharacterSize(22);
    m_songTitle.setFillColor({ 140, 160, 200, 200 });

    m_songTexts.clear();
    for (int i = 0; i < 6; ++i) // 最多 6 首歌
        m_songTexts.emplace_back(m_font);
}

// ═══════════════════════════════════════════════
// 卡片位置
// ═══════════════════════════════════════════════

void PackScene::updateCardTargets()
{
    int n = (int)m_cards.size();
    for (int i = 0; i < n; ++i)
    {
        int offset = i - m_selectedPack;
        m_cards[i].targetX = CENTER_X + offset * CARD_SPACING;
        m_cards[i].targetScale = (offset == 0) ? 1.f : SIDE_SCALE;
        m_cards[i].targetY = (offset == 0) ? CENTER_Y : SIDE_Y;
    }

    // 页码指示器
    std::string dots;
    for (int i = 0; i < n; ++i)
        dots += (i == m_selectedPack) ? " \xe2\x97\x8f  " : " \xe2\x97\x8b  ";
    m_pageDots.setString(dots);
    sf::FloatRect pb = m_pageDots.getLocalBounds();
    m_pageDots.setOrigin({ pb.size.x*.5f, pb.size.y*.5f });
    m_pageDots.setPosition({ 960.f, 740.f });
}

void PackScene::updateSongTexts()
{
    auto& songs = m_packs[m_selectedPack].songs;
    auto& theme = m_packs[m_selectedPack].themeColor;
    for (size_t i = 0; i < m_songTexts.size(); ++i)
    {
        auto& st = m_songTexts[i];
        if (i < songs.size())
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "%s  [%d]", songs[i].title.c_str(), songs[i].difficulty);
            st.setString(buf);
            st.setCharacterSize(22);
            bool sel = (i == m_selectedSong);
            st.setFillColor(sel ? sf::Color(235, 240, 255, 255)
                                : sf::Color(120, 130, 170, 190));
            if (sel)
            {
                st.setOutlineThickness(1.5f);
                st.setOutlineColor({ theme.r, theme.g, theme.b, 180 });
            }
            else
            {
                st.setOutlineThickness(0.f);
            }
            sf::FloatRect b = st.getLocalBounds();
            st.setOrigin({ b.size.x*.5f, b.size.y*.5f });
            st.setPosition({ 960.f, 640.f + i * 36.f });
        }
        else
        {
            st.setString("");
        }
    }
}

std::string PackScene::hintText() const
{
    if (m_navLevel == NavLevel::Pack)
        return "  /\\  \\/  Select    ENTER  Open    ESC  Back";
    else
        return "  /\\  \\/  Select    ENTER  Play    ESC  Back";
}

// ═══════════════════════════════════════════════
// 每帧
// ═══════════════════════════════════════════════

void PackScene::update(float dt)
{
    m_elapsedTime += dt;
    m_glowPulse += dt;

    // 入场动画
    for (size_t i = 0; i < m_cards.size(); ++i)
    {
        float delay = i * 0.1f;
        float localT = (m_elapsedTime - delay) / 0.45f;
        if (localT < 0.f) localT = 0.f;
        if (localT > 1.f) localT = 1.f;
        float eased = Easing::easeOutBack(localT);
        float startX = m_cards[i].targetX + 400.f;
        m_cards[i].currentX = startX + (m_cards[i].targetX - startX) * eased;
        m_cards[i].currentY = m_cards[i].targetY;
        m_cards[i].currentScale = m_cards[i].targetScale;
    }

    updateCards(dt);
    updateSongTexts();

    sf::FloatRect area({ 0, 0 }, { 1920, 1080 });
    m_stars.spawnStars(dt, area, 60);
    m_stars.update(dt);
}

void PackScene::updateCards(float dt)
{
    float speed = 8.f;
    for (auto& card : m_cards)
    {
        card.currentX += (card.targetX - card.currentX) * std::min(speed * dt, 1.f);
        card.currentY += (card.targetY - card.currentY) * std::min(speed * dt, 1.f);
        card.currentScale += (card.targetScale - card.currentScale) * std::min(speed * dt, 1.f);
    }
}

// ═══════════════════════════════════════════════
// 渲染
// ═══════════════════════════════════════════════

void PackScene::render(sf::RenderTarget& target)
{
    target.draw(m_bgGradient);
    m_stars.render(target);

    // ── 顶部栏 ──
    target.draw(m_charLabel);
    target.draw(m_potentialLabel);

    // ── 标题 ──
    m_titleText.setString(m_navLevel == NavLevel::Pack ? "SELECT PACK" : "SELECT SONG");
    target.draw(m_titleText);

    // ── 卡片 ──
    for (size_t i = 0; i < m_cards.size(); ++i)
    {
        auto& cv = m_cards[i];
        auto& pack = m_packs[i];
        bool selected = (i == m_selectedPack && m_navLevel == NavLevel::Pack);

        sf::Transform t;
        t.translate({ cv.currentX, cv.currentY });
        t.scale({ cv.currentScale, cv.currentScale });

        float glowAlpha = selected ? (0.6f + 0.4f * std::sin(m_glowPulse * 2.5f)) : 0.3f;

        if (selected)
        {
            cv.border.setOutlineColor({ pack.themeColor.r, pack.themeColor.g, pack.themeColor.b,
                                        (uint8_t)(glowAlpha * 255.f) });
            cv.border.setOutlineThickness(3.f);
            cv.bg.setFillColor({ 20, 16, 40, 240 });
            cv.nameText.setFillColor({ 235, 240, 255, 255 });
            cv.subText.setFillColor({ 160, 180, 230, 230 });

            sf::RectangleShape glow;
            glow.setSize({ CARD_W + 20.f, CARD_H + 20.f });
            glow.setOrigin({ (CARD_W + 20.f)*.5f, (CARD_H + 20.f)*.5f });
            glow.setFillColor({ pack.themeColor.r, pack.themeColor.g, pack.themeColor.b,
                                (uint8_t)(glowAlpha * 120.f) });
            target.draw(glow, t);
        }
        else
        {
            cv.border.setOutlineColor({ 40, 35, 70, 160 });
            cv.border.setOutlineThickness(2.f);
            cv.bg.setFillColor({ 14, 10, 26, 200 });
            cv.nameText.setFillColor({ 100, 110, 160, 200 });
            cv.subText.setFillColor({ 80, 90, 130, 160 });
        }

        target.draw(cv.bg, t);
        target.draw(cv.border, t);
        cv.nameText.setPosition({ 0.f, -30.f });
        cv.subText.setPosition({ 0.f, 15.f });
        target.draw(cv.nameText, t);
        target.draw(cv.subText, t);

        // 曲目数标记
        if (m_navLevel == NavLevel::Pack)
        {
            char cnt[8];
            snprintf(cnt, sizeof(cnt), "%zu", pack.songs.size());
            sf::Text cntText(m_font, cnt, 14);
            cntText.setFillColor({ 120, 130, 170, 160 });
            sf::FloatRect cb = cntText.getLocalBounds();
            cntText.setOrigin({ cb.size.x*.5f, cb.size.y*.5f });
            cntText.setPosition({ 0.f, CARD_H*.5f - 20.f });
            target.draw(cntText, t);
        }
    }

    // ── 歌曲列表（展开时） ──
    if (m_navLevel == NavLevel::Song)
    {
        auto& theme = m_packs[m_selectedPack].themeColor;
        m_songListBg.setOrigin({ 250.f, 90.f });
        m_songListBg.setPosition({ 960.f, 660.f });
        m_songListBg.setOutlineColor({ theme.r, theme.g, theme.b, 150 });
        target.draw(m_songListBg);

        m_songTitle.setString(m_packs[m_selectedPack].name + "  Tracks");
        sf::FloatRect stb = m_songTitle.getLocalBounds();
        m_songTitle.setOrigin({ stb.size.x*.5f, stb.size.y*.5f });
        m_songTitle.setPosition({ 960.f, 585.f });
        target.draw(m_songTitle);

        for (auto& st : m_songTexts)
            if (st.getString().getSize() > 0)
                target.draw(st);
    }

    // ── 页码 / 提示 ──
    if (m_navLevel == NavLevel::Pack)
        target.draw(m_pageDots);

    float hintAlpha = std::max(0.f, (m_elapsedTime - 1.f) / 0.6f);
    if (hintAlpha > 1.f) hintAlpha = 1.f;
    m_hintText.setString(hintText());
    m_hintText.setCharacterSize(18);
    {
        auto c = m_hintText.getFillColor();
        m_hintText.setFillColor({ 100, 110, 140, (uint8_t)(hintAlpha * 160.f) });
    }
    sf::FloatRect hb = m_hintText.getLocalBounds();
    m_hintText.setOrigin({ hb.size.x*.5f, hb.size.y*.5f });
    m_hintText.setPosition({ 960.f, 900.f });
    target.draw(m_hintText);
}

// ═══════════════════════════════════════════════
// 输入
// ═══════════════════════════════════════════════

void PackScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (m_navLevel == NavLevel::Pack)
        {
            int n = (int)m_packs.size();
            switch (key->code)
            {
            case sf::Keyboard::Key::Left:
            case sf::Keyboard::Key::A:
                m_selectedPack = (m_selectedPack - 1 + n) % n;
                updateCardTargets();
                break;
            case sf::Keyboard::Key::Right:
            case sf::Keyboard::Key::D:
                m_selectedPack = (m_selectedPack + 1) % n;
                updateCardTargets();
                break;
            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space:
                m_navLevel = NavLevel::Song;
                m_selectedSong = 0;
                break;
            case sf::Keyboard::Key::S:
                requestReplace(std::make_unique<SettingsScene>());
                break;
            case sf::Keyboard::Key::Escape:
                requestReplace(std::make_unique<TitleScene>());
                break;
            default: break;
            }
        }
        else // NavLevel::Song
        {
            int ns = (int)m_packs[m_selectedPack].songs.size();
            switch (key->code)
            {
            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::W:
                m_selectedSong = (m_selectedSong - 1 + ns) % ns;
                break;
            case sf::Keyboard::Key::Down:
            case sf::Keyboard::Key::S:
                m_selectedSong = (m_selectedSong + 1) % ns;
                break;
            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space:
                requestReplace(std::make_unique<GameplayScene>());
                break;
            case sf::Keyboard::Key::Escape:
                m_navLevel = NavLevel::Pack;
                m_selectedSong = 0;
                break;
            default: break;
            }
        }
    }
}
