#include "SettingsScene.h"
#include "PackScene.h"
#include <algorithm>
#include <cstdio>

// ═══════════════════════════════════════════════
// 构造
// ═══════════════════════════════════════════════

SettingsScene::SettingsScene()
    : m_titleText(m_font), m_hintText(m_font)
{
    m_font.openFromFile("C:/Windows/Fonts/msyh.ttc") ||
        m_font.openFromFile("C:/Windows/Fonts/arial.ttf");
    buildBackground();
    buildItems();
}

void SettingsScene::onEnter()
{
    m_elapsed = 0.f;
    m_selectedIndex = 0;
    m_stars.clear();
    buildItems(); // 刷新当前值
}

void SettingsScene::onExit() { m_stars.clear(); }

// ═══════════════════════════════════════════════
// 背景
// ═══════════════════════════════════════════════

void SettingsScene::buildBackground()
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

// ═══════════════════════════════════════════════
// 设置项
// ═══════════════════════════════════════════════

void SettingsScene::SettingItem::updateString()
{
    char buf[32];
    if (valuePtr)
    {
        if (step >= 1.f)
            snprintf(buf, sizeof(buf), "%.0f", *valuePtr);
        else
            snprintf(buf, sizeof(buf), "%.1f", *valuePtr);
    }
    valueStr = buf;
}

void SettingsScene::buildItems()
{
    auto& s = GameSettings::inst();

    m_items.clear();

    SettingItem speed;
    speed.label = "Note Speed";
    speed.valuePtr = &s.noteSpeed;
    speed.minVal = 1.0f; speed.maxVal = 3.0f; speed.step = 0.1f;
    speed.unit = "x";
    m_items.push_back(speed);

    SettingItem music;
    music.label = "Music Volume";
    music.valuePtr = &s.musicVol;
    music.minVal = 0; music.maxVal = 100; music.step = 5;
    music.unit = "%";
    m_items.push_back(music);

    SettingItem sfx;
    sfx.label = "SFX Volume";
    sfx.valuePtr = &s.sfxVol;
    sfx.minVal = 0; sfx.maxVal = 100; sfx.step = 5;
    sfx.unit = "%";
    m_items.push_back(sfx);

    SettingItem lat;
    lat.label = "Latency Offset";
    lat.valuePtr = &s.latency;
    lat.minVal = -200; lat.maxVal = 200; lat.step = 10;
    lat.unit = "ms";
    m_items.push_back(lat);

    for (auto& item : m_items) item.updateString();

    // 预创建标签、值文本、滑块
    m_labels.clear(); m_values.clear(); m_tracks.clear(); m_fills.clear();

    for (size_t i = 0; i < m_items.size(); ++i)
    {
        auto& item = m_items[i];
        m_labels.emplace_back(m_font, item.label, 26);
        m_values.emplace_back(m_font, "", 30);
        m_tracks.emplace_back(sf::Vector2f(600.f, 4.f));
        m_fills.emplace_back(sf::Vector2f(0.f, 4.f));
        m_tracks.back().setFillColor({ 30, 30, 50, 200 });
    }

    m_titleText.setString("SETTINGS");
    m_titleText.setCharacterSize(32);
    m_titleText.setFillColor({ 140, 160, 200, 200 });
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin({ tb.size.x*.5f, tb.size.y*.5f });
    m_titleText.setPosition({ 960.f, 100.f });

    m_hintText.setString("  /\\  \\/  Select    <  >  Adjust    ESC  Back");
    m_hintText.setCharacterSize(18);
    m_hintText.setFillColor({ 100, 110, 140, 160 });
}

// ═══════════════════════════════════════════════
// 每帧
// ═══════════════════════════════════════════════

void SettingsScene::update(float dt)
{
    m_elapsed += dt;
    for (auto& item : m_items) item.updateString();

    sf::FloatRect area({ 0, 0 }, { 1920, 1080 });
    m_stars.spawnStars(dt, area, 40);
    m_stars.update(dt);
}

// ═══════════════════════════════════════════════
// 渲染
// ═══════════════════════════════════════════════

void SettingsScene::render(sf::RenderTarget& target)
{
    target.draw(m_bgGradient);
    m_stars.render(target);
    target.draw(m_titleText);

    for (size_t i = 0; i < m_items.size(); ++i)
    {
        auto& item = m_items[i];
        bool sel = (i == m_selectedIndex);
        float barX = 700.f, barY = 325.f + i * 110.f, barW = 600.f, barH = 4.f;

        // 标签
        auto& label = m_labels[i];
        label.setFillColor(sel ? sf::Color(230, 240, 255, 255)
                               : sf::Color(120, 130, 170, 180));
        label.setPosition({ 500.f, 300.f + i * 110.f });
        target.draw(label);

        // 值
        auto& val = m_values[i];
        val.setString(item.valueStr + " " + item.unit);
        val.setFillColor(sel ? sf::Color(100, 200, 255, 255)
                             : sf::Color(160, 170, 200, 200));
        sf::FloatRect vb = val.getLocalBounds();
        val.setPosition({ 1380.f - vb.size.x, 300.f + i * 110.f });
        target.draw(val);

        // 轨道
        auto& track = m_tracks[i];
        track.setPosition({ barX, barY });
        target.draw(track);

        // 填充
        float frac = (*item.valuePtr - item.minVal) / (item.maxVal - item.minVal);
        auto& fill = m_fills[i];
        fill.setSize({ barW * frac, barH });
        fill.setPosition({ barX, barY });
        fill.setFillColor(sel ? sf::Color(100, 200, 255, 200)
                              : sf::Color(80, 120, 180, 150));
        target.draw(fill);

        // 滑块圆点
        if (sel)
        {
            float pulse = 1.f + 0.15f * std::sin(m_elapsed * 3.f);
            float dotR = 7.f * pulse;
            m_dot.setRadius(dotR);
            m_dot.setOrigin({ dotR, dotR });
            m_dot.setPosition({ barX + barW * frac, barY + barH*.5f });
            m_dot.setFillColor({ 180, 230, 255, 220 });
            target.draw(m_dot);
        }
    }

    float ha = std::max(0.f, (m_elapsed - 0.5f) / 0.5f);
    if (ha > 1.f) ha = 1.f;
    auto hc = m_hintText.getFillColor();
    m_hintText.setFillColor({ hc.r, hc.g, hc.b, (uint8_t)(ha * 160.f) });
    sf::FloatRect hb = m_hintText.getLocalBounds();
    m_hintText.setOrigin({ hb.size.x*.5f, hb.size.y*.5f });
    m_hintText.setPosition({ 960.f, 900.f });
    target.draw(m_hintText);
}

// ═══════════════════════════════════════════════
// 输入
// ═══════════════════════════════════════════════

void SettingsScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        int n = (int)m_items.size();
        auto& item = m_items[m_selectedIndex];

        switch (key->code)
        {
        case sf::Keyboard::Key::Up:
        case sf::Keyboard::Key::W:
            m_selectedIndex = (m_selectedIndex - 1 + n) % n;
            break;
        case sf::Keyboard::Key::Down:
        case sf::Keyboard::Key::S:
            m_selectedIndex = (m_selectedIndex + 1) % n;
            break;
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::A:
            if (item.valuePtr)
            {
                *item.valuePtr -= item.step;
                if (*item.valuePtr < item.minVal) *item.valuePtr = item.minVal;
            }
            break;
        case sf::Keyboard::Key::Right:
        case sf::Keyboard::Key::D:
            if (item.valuePtr)
            {
                *item.valuePtr += item.step;
                if (*item.valuePtr > item.maxVal) *item.valuePtr = item.maxVal;
            }
            break;
        case sf::Keyboard::Key::Escape:
            requestReplace(std::make_unique<PackScene>());
            break;
        default: break;
        }
    }
}
