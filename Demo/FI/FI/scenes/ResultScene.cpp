#include "ResultScene.h"
#include "PackScene.h"
#include "../core/Easing.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

// ═══════════════════════════════════════════════
// 构造 & 生命周期
// ═══════════════════════════════════════════════

ResultScene::ResultScene(const GameResult& result)
    : m_result(result)
    , m_titleText(m_font)
    , m_songText(m_font)
    , m_scoreText(m_font)
    , m_comboText(m_font)
    , m_gradeText(m_font)
    , m_hintText(m_font)
{
    m_font.openFromFile("C:/Windows/Fonts/msyh.ttc") ||
        m_font.openFromFile("C:/Windows/Fonts/arial.ttf");

    buildBackground();

    // 评级判定
    if (m_result.maxCombo >= 40)      m_grade = 'S';
    else if (m_result.maxCombo >= 25) m_grade = 'A';
    else if (m_result.maxCombo >= 10) m_grade = 'B';

    // ── 静态文字 ──
    m_titleText.setString("RESULT");
    m_titleText.setCharacterSize(32);
    m_titleText.setFillColor({ 140, 160, 200, 200 });
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin({ tb.size.x * .5f, tb.size.y * .5f });
    m_titleText.setPosition({ 960.f, 100.f });

    m_songText.setString(m_result.songTitle);
    m_songText.setCharacterSize(22);
    m_songText.setFillColor({ 120, 140, 180, 200 });
    sf::FloatRect sb = m_songText.getLocalBounds();
    m_songText.setOrigin({ sb.size.x * .5f, sb.size.y * .5f });
    m_songText.setPosition({ 960.f, 170.f });

    m_scoreText.setCharacterSize(72);
    m_scoreText.setFillColor({ 240, 230, 200, 255 });

    m_comboText.setCharacterSize(26);
    m_comboText.setFillColor({ 180, 190, 220, 200 });

    m_gradeText.setCharacterSize(120);
    m_gradeText.setFillColor(gradeColor());

    m_hintText.setString("ENTER  Continue    ESC  Back");
    m_hintText.setCharacterSize(18);
    m_hintText.setFillColor({ 100, 110, 140, 160 });
    sf::FloatRect hb = m_hintText.getLocalBounds();
    m_hintText.setOrigin({ hb.size.x * .5f, hb.size.y * .5f });
    m_hintText.setPosition({ 960.f, 880.f });
}

void ResultScene::onEnter()
{
    m_elapsed = 0.f;
    m_displayScore = 0.f;
    m_stars.clear();
}

void ResultScene::onExit()
{
    m_stars.clear();
}

// ═══════════════════════════════════════════════
// 背景
// ═══════════════════════════════════════════════

void ResultScene::buildBackground()
{
    constexpr float W = 1920.f, H = 1080.f;
    m_bgGradient.clear();
    m_bgGradient.append({ {0, 0},       {0, 0, 0, 255} });
    m_bgGradient.append({ {W, 0},       {0, 0, 0, 255} });
    m_bgGradient.append({ {0, H * .3f}, {4, 3, 12, 200} });
    m_bgGradient.append({ {W, H * .3f}, {4, 3, 12, 200} });
    m_bgGradient.append({ {0, H * .7f}, {4, 3, 12, 200} });
    m_bgGradient.append({ {W, H * .7f}, {4, 3, 12, 200} });
    m_bgGradient.append({ {0, H},       {0, 0, 0, 255} });
    m_bgGradient.append({ {W, H},       {0, 0, 0, 255} });
}

sf::Color ResultScene::gradeColor() const
{
    switch (m_grade)
    {
    case 'S': return { 255, 220, 80, 255 };
    case 'A': return { 255, 180, 80, 255 };
    case 'B': return { 140, 200, 255, 255 };
    default:  return { 160, 160, 180, 255 };
    }
}

// ═══════════════════════════════════════════════
// 每帧
// ═══════════════════════════════════════════════

void ResultScene::update(float dt)
{
    m_elapsed += dt;

    // ── 分数滚动动画（0.5-1.5s） ──
    float scoreT = (m_elapsed - 0.5f) / 1.0f;
    if (scoreT < 0.f) scoreT = 0.f;
    if (scoreT > 1.f) scoreT = 1.f;
    m_displayScore = (float)m_result.score * Easing::easeOutCubic(scoreT);

    // ── 星空 ──
    sf::FloatRect area({ 0, 0 }, { 1920, 1080 });
    m_stars.spawnStars(dt, area, 40);
    m_stars.update(dt);
}

// ═══════════════════════════════════════════════
// 渲染
// ═══════════════════════════════════════════════

void ResultScene::render(sf::RenderTarget& target)
{
    target.draw(m_bgGradient);
    m_stars.render(target);

    // ── 标题（0.2s 后淡入） ──
    {
        float a = std::min(m_elapsed / 0.4f, 1.f);
        auto c = m_titleText.getFillColor();
        c.a = (uint8_t)(a * 200.f);
        m_titleText.setFillColor(c);
    }
    target.draw(m_titleText);

    // ── 曲名 ──
    {
        float a = std::min((m_elapsed - 0.15f) / 0.4f, 1.f);
        auto c = m_songText.getFillColor();
        c.a = (uint8_t)(a * 200.f);
        m_songText.setFillColor(c);
    }
    target.draw(m_songText);

    // ── 分数 ──
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", (int)m_displayScore);
        m_scoreText.setString(buf);
        sf::FloatRect sb = m_scoreText.getLocalBounds();
        m_scoreText.setOrigin({ sb.size.x * .5f, sb.size.y * .5f });
        m_scoreText.setPosition({ 960.f, 340.f });

        float a = std::min((m_elapsed - 0.4f) / 0.5f, 1.f);
        auto c = m_scoreText.getFillColor();
        c.a = (uint8_t)(a * 255.f);
        m_scoreText.setFillColor(c);
    }
    target.draw(m_scoreText);

    // ── 连击 ──
    {
        char buf[48];
        snprintf(buf, sizeof(buf), "Max Combo: %d", m_result.maxCombo);
        m_comboText.setString(buf);
        sf::FloatRect cb = m_comboText.getLocalBounds();
        m_comboText.setOrigin({ cb.size.x * .5f, cb.size.y * .5f });
        m_comboText.setPosition({ 960.f, 460.f });

        float a = std::min((m_elapsed - 0.8f) / 0.5f, 1.f);
        auto c = m_comboText.getFillColor();
        c.a = (uint8_t)(a * 200.f);
        m_comboText.setFillColor(c);
    }
    target.draw(m_comboText);

    // ── 评级（1.2s 后弹出） ──
    {
        float gt = (m_elapsed - 1.2f) / 0.5f;
        if (gt < 0.f) gt = 0.f;
        if (gt > 1.f) gt = 1.f;
        float scale = Easing::easeOutBack(gt);

        char gbuf[4];
        snprintf(gbuf, sizeof(gbuf), "%c", m_grade);
        m_gradeText.setString(gbuf);
        m_gradeText.setScale({ scale, scale });
        sf::FloatRect gb = m_gradeText.getLocalBounds();
        m_gradeText.setOrigin({ gb.size.x * .5f, gb.size.y * .5f });
        m_gradeText.setPosition({ 960.f, 590.f });

        auto c = gradeColor();
        c.a = (uint8_t)(gt * 255.f);
        m_gradeText.setFillColor(c);
    }
    target.draw(m_gradeText);

    // ── 提示 ──
    {
        float a = std::min((m_elapsed - 2.5f) / 0.6f, 1.f);
        auto c = m_hintText.getFillColor();
        c.a = (uint8_t)(a * 160.f);
        m_hintText.setFillColor(c);
    }
    target.draw(m_hintText);
}

// ═══════════════════════════════════════════════
// 输入
// ═══════════════════════════════════════════════

void ResultScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Enter ||
            key->code == sf::Keyboard::Key::Space ||
            key->code == sf::Keyboard::Key::Escape)
        {
            requestReplace(std::make_unique<PackScene>());
        }
    }
}
