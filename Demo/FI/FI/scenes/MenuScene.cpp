#include "MenuScene.h"
#include "TitleScene.h"
#include "PackScene.h"
#include "../core/Easing.h"
#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════
// 构造 & 生命周期
// ═══════════════════════════════════════════════

MenuScene::MenuScene()
    : m_titleText(m_font)
    , m_hintText(m_font)
{
    m_fontLoaded = m_font.openFromFile("C:/Windows/Fonts/msyh.ttc");
    if (!m_fontLoaded)
        m_fontLoaded = m_font.openFromFile("C:/Windows/Fonts/arial.ttf");

    buildBackground();
    buildMenuItems();
}

void MenuScene::onEnter()
{
    m_elapsedTime = 0.f;
    m_selectedIndex = 0;
    m_previousIndex = 0;
    m_indicatorY = m_items[0].targetY;
    m_indicatorTargetY = m_items[0].targetY;

    // 重置所有菜单项的入场位置（从右侧屏外开始）
    for (auto& item : m_items)
        item.currentX = item.targetX + 300.f;

    m_stars.clear();
}

void MenuScene::onExit()
{
    m_stars.clear();
}

// ═══════════════════════════════════════════════
// 背景构建（一次性）
// ═══════════════════════════════════════════════

void MenuScene::buildBackground()
{
    // 暗角渐变 — 同 TitleScene 风格
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

// ═══════════════════════════════════════════════
// 菜单项构建（一次性）
// ═══════════════════════════════════════════════

void MenuScene::buildMenuItems()
{
    // ── 顶部标题 ──
    m_titleText.setString("ABYSSAL BEAT");
    m_titleText.setCharacterSize(28);
    m_titleText.setFillColor({ 120, 140, 180, 180 });
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin({ tb.size.x * .5f, tb.size.y * .5f });
    m_titleText.setPosition({ 960.f, 180.f });

    // ── 菜单项 ──
    // 每个菜单项的 targetY 按 100px 间距排列，从 440 开始
    struct { const char* label; float y; } defs[] = {
        { "START GAME", 440.f },
        { "EXIT",       540.f },
    };

    m_items.clear();
    m_itemTexts.clear();
    constexpr float MENU_X = 960.f;

    for (auto& def : defs)
    {
        MenuItem item;
        item.label = def.label;
        item.targetX = MENU_X;
        item.targetY = def.y;
        item.currentX = MENU_X + 300.f; // 初始在右侧屏外
        item.entryDelay = m_items.size() * 0.12f;
        m_items.push_back(item);

        sf::Text text(m_font);
        text.setString(def.label);
        text.setCharacterSize(48);
        text.setFillColor({ 100, 120, 170, 255 });
        text.setOutlineThickness(0.f);
        sf::FloatRect lb = text.getLocalBounds();
        text.setOrigin({ lb.size.x * .5f, lb.size.y * .5f });
        m_itemTexts.push_back(std::move(text));
    }

    // ── 底部提示 ──
    m_hintText.setString("  /\\  \\/  Navigate    ENTER  Confirm    ESC  Back");
    m_hintText.setCharacterSize(18);
    m_hintText.setFillColor({ 100, 110, 140, 160 });
    sf::FloatRect hb = m_hintText.getLocalBounds();
    m_hintText.setOrigin({ hb.size.x * .5f, hb.size.y * .5f });
    m_hintText.setPosition({ 960.f, 880.f });
}

// ═══════════════════════════════════════════════
// 每帧逻辑
// ═══════════════════════════════════════════════

void MenuScene::update(float dt)
{
    m_elapsedTime += dt;

    // ── 入场动画：菜单项 staggered 滑入 ──
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        float localT = m_elapsedTime - m_items[i].entryDelay;
        if (localT < 0.f) localT = 0.f;
        float t = localT / 0.5f; // 0.5s 滑入时长
        if (t > 1.f) t = 1.f;

        float offset = (1.f - Easing::easeOutBack(t)) * 300.f;
        m_items[i].currentX = m_items[i].targetX + offset;
    }

    // ── 菜单项位置更新 ──
    updateItemPositions();

    // ── 指示器平滑移动 ──
    updateIndicator(dt);

    // ── 星空粒子 ──
    sf::FloatRect area({ 0, 0 }, { 1920, 1080 });
    m_stars.spawnStars(dt, area, 80);
    m_stars.update(dt);
}

void MenuScene::updateItemPositions()
{
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        bool selected = (i == m_selectedIndex);
        auto& text = m_itemTexts[i];
        auto& item = m_items[i];

        text.setPosition({ item.currentX, item.targetY });

        // 选中态 vs 未选中态颜色平滑过渡（直接 snap，无需动画）
        if (selected)
        {
            text.setFillColor({ 230, 240, 255, 255 });
            text.setOutlineThickness(3.f);
            text.setOutlineColor({ 60, 150, 240, 200 });
            text.setCharacterSize(52); // 选中稍大
        }
        else
        {
            text.setFillColor({ 80, 100, 150, 200 });
            text.setOutlineThickness(0.f);
            text.setOutlineColor(sf::Color::Transparent);
            text.setCharacterSize(48);
        }

        // 重新设置 origin（因为字号可能变了）
        sf::FloatRect lb = text.getLocalBounds();
        text.setOrigin({ lb.size.x * .5f, lb.size.y * .5f });
    }
}

void MenuScene::updateIndicator(float dt)
{
    m_indicatorTargetY = m_items[m_selectedIndex].targetY;

    // 平滑插值（指数衰减，更自然）
    float diff = m_indicatorTargetY - m_indicatorY;
    float speed = 12.f; // 跟随速度
    m_indicatorY += diff * std::min(speed * dt, 1.f);
}

// ═══════════════════════════════════════════════
// 渲染
// ═══════════════════════════════════════════════

void MenuScene::render(sf::RenderTarget& target)
{
    // ── 背景 ──
    target.draw(m_bgGradient);
    m_stars.render(target);

    // ── 顶部标题 ──
    target.draw(m_titleText);

    // ── 选中指示器 ▶ ──
    // 根据当前指示器 Y 动态绘制三角
    {
        float iy = m_indicatorY;
        float ix = m_items[m_selectedIndex].currentX - 250.f; // 在菜单项左侧

        // 指示器颜色随入场进度渐显
        float entryAlpha = std::min(m_elapsedTime / 0.6f, 1.f);
        uint8_t ia = (uint8_t)(entryAlpha * 220.f);
        sf::Color ic(120, 200, 255, ia);

        sf::VertexArray tri{ sf::PrimitiveType::Triangles };
        tri.append({ { ix, iy - 12.f }, ic });
        tri.append({ { ix + 16.f, iy }, ic });
        tri.append({ { ix, iy + 12.f }, ic });
        target.draw(tri);
    }

    // ── 菜单项文字 ──
    for (auto& text : m_itemTexts)
        target.draw(text);

    // ── 底部提示 ──
    float hintAlpha = std::max(0.f, (m_elapsedTime - 1.5f) / 1.f);
    if (hintAlpha > 1.f) hintAlpha = 1.f;
    {
        auto c = m_hintText.getFillColor();
        c.a = (uint8_t)(hintAlpha * 160.f);
        m_hintText.setFillColor(c);
    }
    target.draw(m_hintText);
}

// ═══════════════════════════════════════════════
// 输入处理
// ═══════════════════════════════════════════════

void MenuScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        int count = (int)m_items.size();
        if (count == 0) return;

        switch (key->code)
        {
        case sf::Keyboard::Key::Up:
        case sf::Keyboard::Key::W:
            m_previousIndex = m_selectedIndex;
            m_selectedIndex = (m_selectedIndex - 1 + count) % count;
            break;

        case sf::Keyboard::Key::Down:
        case sf::Keyboard::Key::S:
            m_previousIndex = m_selectedIndex;
            m_selectedIndex = (m_selectedIndex + 1) % count;
            break;

        case sf::Keyboard::Key::Enter:
        case sf::Keyboard::Key::Space:
            if (m_selectedIndex == 0)
            {
                // START GAME → PackScene
                requestReplace(std::make_unique<PackScene>());
            }
            else if (m_selectedIndex == 1)
            {
                // EXIT → 发送关闭事件
                // SFML 没有直接 close 的跨场景方法，用 event push
                // 简单方案：不做，用 ESC 返回标题后关闭
            }
            break;

        case sf::Keyboard::Key::Escape:
            requestReplace(std::make_unique<TitleScene>());
            break;

        default:
            break;
        }
    }
}
