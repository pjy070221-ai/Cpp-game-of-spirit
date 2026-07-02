#include "TitleScene.h"
#include "PackScene.h"
#include "../core/Easing.h"
#include <algorithm>
#include <cmath>
#include <random>

static std::mt19937& rng() { static std::mt19937 gen(42); return gen; }

// ═══════════════════════════════════════════════
// 构造 & 生命周期
// ═══════════════════════════════════════════════

TitleScene::TitleScene()
    : m_promptText(m_font)
    , m_loadLabel(m_font)
    , m_loadPercent(m_font)
{
    m_fontLoaded = m_font.openFromFile("C:/Windows/Fonts/msyh.ttc");
    if (!m_fontLoaded)
        m_fontLoaded = m_font.openFromFile("C:/Windows/Fonts/arial.ttf");

    bool ok = m_textLayer.resize({ 1920, 1080 }); (void)ok;

    // 背景（构造一次，按状态选择性绘制）
    buildShatteredBackground();
    buildEnergyGlow();
    buildFlashQuad();
    buildCharTexts(L"Abyssal Beat", m_titleChars, 380.f, 60.f, 90.f);
    buildCharTexts(L"Demo", m_subChars, 510.f, 24.f, 36.f);

    // 预热 RenderTexture + 字形光栅化 — 避免 renderNormal 首帧卡顿
    // SFML 在首次 draw 时会用 FreeType 光栅化每个字形（微软雅黑尤其重），
    // 提前做一次让所有字形进入缓存，同时让 GPU 初始化 FBO
    m_textLayer.clear(sf::Color::Transparent);
    for (auto& cd : m_titleChars) m_textLayer.draw(cd.text);
    for (auto& cd : m_subChars) m_textLayer.draw(cd.text);
    m_textLayer.display();

    // 入场动画系统
    buildLoadingBar();
    buildShatterCracks();

    // 提示文字
    m_promptText.setString(L"Press ENTER to Start");
    m_promptText.setCharacterSize(26);
    m_promptText.setFillColor({ 200, 200, 210, 0 });

    std::uniform_real_distribution<float> next(2.f, 5.f);
    m_glitchTimer = next(rng());
}

void TitleScene::onEnter()
{
    m_elapsedTime = 0.f;
    m_globalAlpha = m_subAlpha = m_promptAlpha = 0.f;
    m_promptBlinkTimer = 0.f;
    m_flashAlpha = 0.f;
    m_glitchTimer = 2.f + (rng()() % 300) * .01f;
    m_glitchDuration = m_glitchIntensity = 0.f;
    m_stars.clear(); m_dust.clear();

    // 重置入场状态机
    m_state = State::Loading;
    m_loadProgress = 0.f;
    m_loadRaw = 0.f;
    m_shatterTimer = 0.f;
    m_shardAlpha = 0.f;
    m_shatterFlashAlpha = 0.f;
    m_titleStartTime = 0.f;
}

void TitleScene::onExit() { m_stars.clear(); m_dust.clear(); }

void TitleScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space)
        {
            // 仅在 Idle 态响应（入场动画期间忽略）
            if (m_state == State::Idle)
                requestReplace(std::make_unique<PackScene>());
        }
    }
}

// ═══════════════════════════════════════════════
// 进度条构建（一次性）
// ═══════════════════════════════════════════════
void TitleScene::buildLoadingBar()
{
    constexpr float BAR_W = 480.f, BAR_H = 6.f;
    constexpr float BAR_X = 960.f - BAR_W * .5f;
    constexpr float BAR_Y = 600.f;

    // 轨道 — 深灰底色
    m_loadTrack.setSize({ BAR_W, BAR_H });
    m_loadTrack.setPosition({ BAR_X, BAR_Y });
    m_loadTrack.setFillColor({ 20, 20, 30, 180 });

    // 填充条 — 初始宽度 0，后面每帧更新
    m_loadFill.setSize({ 0.f, BAR_H });
    m_loadFill.setPosition({ BAR_X, BAR_Y });
    m_loadFill.setFillColor({ 60, 180, 255, 255 });

    // 发光光晕 — 填充条背后的大尺寸半透明条
    m_loadGlow.clear();
    constexpr float GLOW_H = 16.f;
    constexpr float GY = BAR_Y - (GLOW_H - BAR_H) * .5f;
    sf::Color glowC(60, 160, 255, 25);
    sf::Color glowT(60, 160, 255, 0);
    // 光晕 = 2 个三角组成矩形
    m_loadGlow.append({ { BAR_X, GY }, glowT });
    m_loadGlow.append({ { BAR_X + BAR_W, GY }, glowT });
    m_loadGlow.append({ { BAR_X, GY + GLOW_H }, glowC });
    m_loadGlow.append({ { BAR_X + BAR_W, GY }, glowT });
    m_loadGlow.append({ { BAR_X + BAR_W, GY + GLOW_H }, glowC });
    m_loadGlow.append({ { BAR_X, GY + GLOW_H }, glowC });

    // 标签 "LOADING..."
    m_loadLabel.setString("LOADING...");
    m_loadLabel.setCharacterSize(16);
    m_loadLabel.setFillColor({ 140, 160, 200, 200 });
    sf::FloatRect lb = m_loadLabel.getLocalBounds();
    m_loadLabel.setOrigin({ lb.size.x * .5f, lb.size.y + 8.f });
    m_loadLabel.setPosition({ 960.f, BAR_Y - 6.f });

    // 百分比（右侧）
    m_loadPercent.setCharacterSize(16);
    m_loadPercent.setFillColor({ 160, 200, 240, 220 });
    m_loadPercent.setPosition({ BAR_X + BAR_W + 12.f, BAR_Y - 10.f });
}

// ═══════════════════════════════════════════════
// 破裂裂纹构建（一次性）
// ═══════════════════════════════════════════════
void TitleScene::buildShatterCracks()
{
    constexpr int NUM_CRACKS = 52;
    m_shatterCrackData.clear();
    m_shatterCrackGeom.clear();

    std::uniform_real_distribution<float> angleJitter(-0.12f, 0.12f); // ±7°
    std::uniform_real_distribution<float> lenRand(250.f, 1150.f);
    std::uniform_real_distribution<float> delayRand(0.f, 0.55f);

    for (int i = 0; i < NUM_CRACKS; ++i)
    {
        float baseAngle = 2.f * 3.14159265f * i / NUM_CRACKS;
        ShatterCrack sc;
        sc.angle = baseAngle + angleJitter(rng());
        sc.maxLen = lenRand(rng());
        sc.delay = delayRand(rng());
        m_shatterCrackData.push_back(sc);
    }

    // 按 delay 排序 — 短裂纹优先（产生"从中心向外扩散"的节奏感）
    std::sort(m_shatterCrackData.begin(), m_shatterCrackData.end(),
        [](const ShatterCrack& a, const ShatterCrack& b) { return a.delay < b.delay; });

    // 预分配顶点：每个裂纹 2 个顶点（start + end）
    m_shatterCrackGeom.resize(NUM_CRACKS * 2);
    sf::Color crackColor(180, 200, 255, 200);
    for (int i = 0; i < NUM_CRACKS; ++i)
    {
        m_shatterCrackGeom[i * 2].position = m_impactPoint;
        m_shatterCrackGeom[i * 2].color = crackColor;
        // 端点 position 每帧更新，color 预设
        m_shatterCrackGeom[i * 2 + 1].position = m_impactPoint; // 初始长度为 0
        m_shatterCrackGeom[i * 2 + 1].color = sf::Color(120, 160, 240, 160);
    }
}

// ═══════════════════════════════════════════════
// 状态机 — update 分发
// ═══════════════════════════════════════════════
void TitleScene::update(float dt)
{
    m_elapsedTime += dt;

    switch (m_state)
    {
    case State::Loading:     updateLoading(dt);     break;
    case State::Shattering:  updateShattering(dt);  break;
    case State::TitleReveal: updateTitleReveal(dt); break;
    case State::Idle:        updateIdle(dt);        break;
    }

    // 粒子系统 — 所有阶段都运行（Loading 时也保持画面生动）
    sf::FloatRect area({ 0, 0 }, { 1920, 1080 });
    m_stars.spawnStars(dt, area, 100);
    m_stars.update(dt);

    static float dustT = 0.f;
    dustT += dt;
    if (dustT > .25f) {
        dustT = 0.f;
        std::uniform_real_distribution<float> xd(0, 1920), yd(0, 1080);
        m_dust.emit({ xd(rng()), yd(rng()) }, 4, { 50, 60, 100, 25 }, 4, 12, 2.5f, 6, 2, 7);
    }
    m_dust.update(dt);
}

// ── Loading 阶段 ──
void TitleScene::updateLoading(float dt)
{
    // 原始线性进度独立推进（不受 easing 反馈影响）
    m_loadRaw += dt / LOAD_DURATION;

    // 最后 10% 微微加速（easeInCubic），但 raw 值不受影响
    if (m_loadRaw > 0.9f)
    {
        float t = (m_loadRaw - 0.9f) / 0.1f; // 0→1 in last 10%
        if (t > 1.f) t = 1.f;
        m_loadProgress = 0.9f + Easing::easeInCubic(t) * 0.1f;
    }
    else
    {
        m_loadProgress = m_loadRaw;
    }

    if (m_loadRaw >= 1.f)
    {
        m_loadProgress = 1.f;
        m_state = State::Shattering;
        m_shatterTimer = 0.f;
    }
}

// ── Shattering 阶段 ──
void TitleScene::updateShattering(float dt)
{
    m_shatterTimer += dt;
    float t = m_shatterTimer / SHATTER_DURATION; // 0→1

    // 白闪：0~0.15s 快速出现然后衰减
    if (m_shatterTimer < 0.15f)
        m_shatterFlashAlpha = (1.f - m_shatterTimer / 0.15f) * 0.55f;
    else
        m_shatterFlashAlpha = 0.f;

    // 更新每条裂纹的端点位置
    for (size_t i = 0; i < m_shatterCrackData.size(); ++i)
    {
        const auto& sc = m_shatterCrackData[i];
        float localT = 0.f;
        if (t > sc.delay)
        {
            float raw = (t - sc.delay) / (1.f - sc.delay);
            localT = Easing::easeOutCubic(raw);
        }
        float len = sc.maxLen * localT;
        float ex = m_impactPoint.x + std::cos(sc.angle) * len;
        float ey = m_impactPoint.y + std::sin(sc.angle) * len;
        m_shatterCrackGeom[i * 2 + 1].position = { ex, ey };
    }

    // 碎裂背景淡入（裂纹扩散到 ~30% 后开始）
    float shardT = (t - 0.25f) / 0.55f; // 0.25→0.8 映射到 0→1
    if (shardT < 0.f) shardT = 0.f;
    if (shardT > 1.f) shardT = 1.f;
    m_shardAlpha = Easing::easeOutCubic(shardT);

    // 过渡到 TitleReveal
    if (m_shatterTimer >= SHATTER_DURATION)
    {
        m_shardAlpha = 1.f;
        m_shatterFlashAlpha = 0.f;
        m_state = State::TitleReveal;
        m_titleStartTime = m_elapsedTime;
    }
}

// ── TitleReveal 阶段（复用原入场动画逻辑） ──
void TitleScene::updateTitleReveal(float dt)
{
    float t = m_elapsedTime - m_titleStartTime; // 相对于本阶段起点的时间

    // 入场透明度
    float t1 = (t < 0.f ? 0.f : (t > 1.5f ? 1.f : t / 1.5f));
    m_globalAlpha = Easing::easeOutExpo(t1);

    float t2 = (t - 1.f < 0.f ? 0.f : (t - 1.f > 1.5f ? 1.f : (t - 1.f) / 1.5f));
    m_subAlpha = Easing::easeOutCubic(t2);

    float t3 = (t - 2.5f < 0.f ? 0.f : (t - 2.5f > .8f ? 1.f : (t - 2.5f) / .8f));
    m_promptAlpha = Easing::easeOutCubic(t3);

    if (t > 3.3f) {
        m_promptBlinkTimer += dt;
        m_promptAlpha = (std::sin(m_promptBlinkTimer * 3.f) * .5f + .5f);
        if (m_promptAlpha < .2f) m_promptAlpha = .2f;
        if (m_promptAlpha > 1.f) m_promptAlpha = 1.f;
    }

    // 标题单字动画
    float baseY = m_titleStartY + (m_titleTargetY - m_titleStartY) * Easing::easeOutBack(t1);
    for (auto& cd : m_titleChars) {
        float ph = cd.animPhase + t;
        float fy = std::sin(ph * 1.7f) * 3.f + std::cos(ph * 2.3f) * 2.f;
        float sw = 1.f + std::sin(ph * .9f) * cd.sizeVariance * .5f;
        cd.text.setCharacterSize((unsigned)(cd.baseSize * sw));
        cd.text.setPosition({ cd.posX, baseY + fy });
        auto c = cd.text.getFillColor();
        c.a = (uint8_t)(m_globalAlpha * 255);
        cd.text.setFillColor(c);
        auto oc = cd.text.getOutlineColor();
        oc.a = (uint8_t)(m_globalAlpha * 160);
        cd.text.setOutlineColor(oc);
    }

    // 副标题动画
    for (auto& cd : m_subChars) {
        float ph = cd.animPhase + t * .6f;
        float fy = std::sin(ph * 1.2f) * 2.f;
        cd.text.setPosition({ cd.posX, cd.posY + fy });
        auto c = cd.text.getFillColor();
        c.a = (uint8_t)(m_subAlpha * 180);
        cd.text.setFillColor(c);
    }

    // 提示
    {
        auto pc = m_promptText.getFillColor();
        pc.a = (uint8_t)(m_promptAlpha * 255);
        m_promptText.setFillColor(pc);
        sf::FloatRect pb = m_promptText.getLocalBounds();
        m_promptText.setOrigin({ pb.size.x * .5f, pb.size.y * .5f });
        m_promptText.setPosition({ 960.f, 730.f });
    }

    // 标题动画完成后进入 Idle
    if (t > 5.5f)
    {
        m_state = State::Idle;
        std::uniform_real_distribution<float> next(2.f, 5.f);
        m_glitchTimer = next(rng());
    }
}

// ── Idle 阶段（复用原故障抽搐逻辑） ──
void TitleScene::updateIdle(float dt)
{
    // 故障抽搐
    if (m_glitchDuration > 0.f) {
        m_glitchDuration -= dt;
        m_flashAlpha = m_glitchIntensity * (m_glitchDuration / 0.27f);
        if (m_glitchDuration <= 0.f) {
            m_glitchDuration = 0.f;
            m_glitchIntensity = 0.f;
            m_flashAlpha = 0.f;
            std::uniform_real_distribution<float> next(2.f, 5.f);
            m_glitchTimer = next(rng());
        }
    }
    else {
        m_glitchTimer -= dt;
        if (m_glitchTimer <= 0.f) triggerGlitch();
    }

    // 中心能量脉动（只改 1 个顶点的 alpha）
    {
        float pulse = 0.5f + 0.5f * std::sin(m_elapsedTime * 0.9f);
        pulse += m_glitchIntensity * 0.5f;
        float alpha = pulse < 0.3f ? 0.3f : (pulse > 1.f ? 1.f : pulse);
        uint8_t a = (uint8_t)(alpha * 80.f);
        m_bgEnergy[0].color = sf::Color(16, 20, 80, a);
    }

    // 闪白覆盖
    {
        uint8_t fa = (uint8_t)(m_flashAlpha * 40.f);
        sf::Color fc(255, 240, 255, fa);
        for (size_t i = 0; i < m_bgFlash.getVertexCount(); ++i)
            m_bgFlash[i].color = fc;
    }

    // 裂纹呼吸
    animateCracks(m_elapsedTime);

    // 标题单字动画（Idle 态：无入场位移，只有浮动 + 故障抖动）
    float totalT = m_elapsedTime;
    for (auto& cd : m_titleChars) {
        float ph = cd.animPhase + totalT;
        float fy = std::sin(ph * 1.7f) * 3.f + std::cos(ph * 2.3f) * 2.f;
        float gx = 0.f, gy = 0.f;
        if (m_glitchIntensity > 0.f) {
            float gp = ph * 40.f;
            gx = std::sin(gp) * 8.f * m_glitchIntensity + std::cos(gp * 2.7f) * 5.f * m_glitchIntensity;
            gy = std::cos(gp * 1.3f) * 6.f * m_glitchIntensity + std::sin(gp * 3.1f) * 4.f * m_glitchIntensity;
        }
        float sw = 1.f + std::sin(ph * .9f) * cd.sizeVariance * .5f;
        cd.text.setCharacterSize((unsigned)(cd.baseSize * sw));
        cd.text.setPosition({ cd.posX + gx, cd.posY + fy + gy });
        auto c = cd.text.getFillColor();
        c.a = (uint8_t)(m_globalAlpha * 255);
        cd.text.setFillColor(c);
        auto oc = cd.text.getOutlineColor();
        oc.a = (uint8_t)(m_globalAlpha * 160);
        cd.text.setOutlineColor(oc);
    }

    for (auto& cd : m_subChars) {
        float ph = cd.animPhase + totalT * .6f;
        float fy = std::sin(ph * 1.2f) * 2.f;
        float gx = 0.f, gy = 0.f;
        if (m_glitchIntensity > 0.f) {
            gx = std::sin(ph * 30.f) * 3.f * m_glitchIntensity;
            gy = std::cos(ph * 25.f) * 2.f * m_glitchIntensity;
        }
        cd.text.setPosition({ cd.posX + gx, cd.posY + fy + gy });
        auto c = cd.text.getFillColor();
        c.a = (uint8_t)(m_subAlpha * 180);
        cd.text.setFillColor(c);
    }

    // 提示呼吸
    {
        m_promptBlinkTimer += dt;
        float a = std::sin(m_promptBlinkTimer * 3.f) * .5f + .5f;
        if (a < .2f) a = .2f; if (a > 1.f) a = 1.f;
        auto pc = m_promptText.getFillColor();
        pc.a = (uint8_t)(a * 255);
        m_promptText.setFillColor(pc);
        sf::FloatRect pb = m_promptText.getLocalBounds();
        m_promptText.setOrigin({ pb.size.x * .5f, pb.size.y * .5f });
        m_promptText.setPosition({ 960.f, 730.f });
    }
}

// ═══════════════════════════════════════════════
// 状态机 — render 分发
// ═══════════════════════════════════════════════
void TitleScene::render(sf::RenderTarget& target)
{
    switch (m_state)
    {
    case State::Loading:    renderLoading(target);    break;
    case State::Shattering: renderShattering(target); break;
    default:                renderNormal(target);     break; // TitleReveal + Idle
    }
}

// ── Loading 渲染 ──
// 注：Application::render() 已调用 window.clear(sf::Color::Black)
void TitleScene::renderLoading(sf::RenderTarget& target)
{
    // 粒子（星空 + 浮尘）
    m_dust.render(target);
    m_stars.render(target);

    // 进度条光晕（只有已填充部分才发光）
    float fillW = 480.f * m_loadProgress;
    if (fillW > 0.f)
    {
        // 动态调整光晕宽度
        constexpr float BAR_X = 960.f - 480.f * .5f;
        constexpr float GLOW_H = 16.f, BAR_H = 6.f, BAR_Y = 600.f;
        constexpr float GY = BAR_Y - (GLOW_H - BAR_H) * .5f;
        sf::Color glowC(60, 160, 255, 30);
        sf::Color glowT(60, 160, 255, 0);

        sf::VertexArray glow{ sf::PrimitiveType::Triangles };
        glow.append({ { BAR_X, GY }, glowT });
        glow.append({ { BAR_X + fillW, GY }, glowT });
        glow.append({ { BAR_X, GY + GLOW_H }, glowC });
        glow.append({ { BAR_X + fillW, GY }, glowT });
        glow.append({ { BAR_X + fillW, GY + GLOW_H }, glowC });
        glow.append({ { BAR_X, GY + GLOW_H }, glowC });
        target.draw(glow);
    }

    // 轨道
    target.draw(m_loadTrack);

    // 填充条
    m_loadFill.setSize({ fillW, 6.f });
    target.draw(m_loadFill);

    // 百分比文字
    int pct = (int)(m_loadProgress * 100.f);
    m_loadPercent.setString(std::to_string(pct) + "%");
    target.draw(m_loadPercent);

    // LOADING... 标签
    target.draw(m_loadLabel);
}

// ── Shattering 渲染 ──
// 注：Application::render() 已调用 window.clear(sf::Color::Black)
void TitleScene::renderShattering(sf::RenderTarget& target)
{
    // 裂纹（从中心扩散的直线）
    target.draw(m_shatterCrackGeom);

    // 碎裂背景 + 暗角（按 m_shardAlpha 淡入）
    if (m_shardAlpha > 0.001f)
    {
        // 用 shader 做不到 per-draw alpha 控制时，直接改顶点颜色做淡入
        // 偷懒做法：先用全黑清屏覆盖，再以 alpha 混合绘制背景
        // 更简单的做法：直接在碎裂背景后面画一层黑色，降低其不透明度

        // 先画一层半透明黑来"压暗"，再用 shardAlpha 控制背景可见度
        // 实际上 SFML 的 draw 本身不支持全局 alpha，这里我们用一个取巧方案：
        // 画一个全屏黑色 Quad 在背景上面，其透明度 = 1 - m_shardAlpha
        uint8_t coverA = (uint8_t)((1.f - m_shardAlpha) * 255.f);
        sf::VertexArray cover{ sf::PrimitiveType::Triangles };
        cover.append({ {0, 0},       {4, 3, 10, coverA} });
        cover.append({ {1920, 0},    {4, 3, 10, coverA} });
        cover.append({ {0, 1080},    {4, 3, 10, coverA} });
        cover.append({ {1920, 0},    {4, 3, 10, coverA} });
        cover.append({ {1920, 1080}, {4, 3, 10, coverA} });
        cover.append({ {0, 1080},    {4, 3, 10, coverA} });

        // 先画碎裂背景
        target.draw(m_bgShards);
        target.draw(m_bgGradient);
        // 再在上面盖一层衰减黑幕（alpha 从 255→0，即背景从不可见到完全可见）
        target.draw(cover);
    }

    // 白闪
    if (m_shatterFlashAlpha > 0.001f)
    {
        sf::VertexArray flash{ sf::PrimitiveType::Triangles };
        uint8_t fa = (uint8_t)(m_shatterFlashAlpha * 255.f);
        sf::Color fc(220, 230, 255, fa);
        flash.append({ {0, 0}, fc });
        flash.append({ {1920, 0}, fc });
        flash.append({ {0, 1080}, fc });
        flash.append({ {1920, 0}, fc });
        flash.append({ {1920, 1080}, fc });
        flash.append({ {0, 1080}, fc });
        target.draw(flash);
    }

    // 粒子（贯穿所有阶段）
    m_dust.render(target);
    m_stars.render(target);
}

// ── 完整渲染（TitleReveal + Idle，同原逻辑） ──
void TitleScene::renderNormal(sf::RenderTarget& target)
{
    // ══ 背景静态几何 ══
    target.draw(m_bgShards);
    target.draw(m_bgGradient);

    // ══ 中心能量脉动 ══
    target.draw(m_bgEnergy);

    // ══ 裂纹（颜色每帧更新） ══
    target.draw(m_bgCracks);

    // ══ 粒子 ══
    m_dust.render(target);
    m_stars.render(target);

    // ══ 文字预渲染 → RGB 分裂 ══
    m_textLayer.clear(sf::Color::Transparent);
    for (auto& cd : m_titleChars) m_textLayer.draw(cd.text);
    for (auto& cd : m_subChars) m_textLayer.draw(cd.text);
    m_textLayer.display();

    const sf::Texture& tex = m_textLayer.getTexture();
    sf::Sprite sp(tex);
    float split = 4.f + std::sin(m_elapsedTime * 7.3f) * 1.2f + std::cos(m_elapsedTime * 11.1f) * .8f;
    float jitY = std::sin(m_elapsedTime * 15.7f) * 1.5f;

    sp.setColor({ 255, 60, 60, (uint8_t)(m_globalAlpha * 140) });
    sp.setPosition({ -split, jitY });
    target.draw(sp);

    sp.setColor({ 50, 70, 255, (uint8_t)(m_globalAlpha * 130) });
    sp.setPosition({ split, -jitY });
    target.draw(sp);

    sp.setColor({ 255, 255, 255, (uint8_t)(m_globalAlpha * 255) });
    sp.setPosition({ 0, 0 });
    target.draw(sp);

    // ══ 故障闪白 ══
    if (m_flashAlpha > 0.001f)
        target.draw(m_bgFlash);

    // ══ 提示 ══
    target.draw(m_promptText);
}

// ═══════════════════════════════════════════════
// 以下为原有的一次性构建函数（未修改）
// ═══════════════════════════════════════════════

void TitleScene::buildShatteredBackground()
{
    constexpr int COLS = 32, ROWS = 19;
    constexpr float W = 1920.f, H = 1080.f, CW = W / COLS, RH = H / ROWS;

    std::uniform_real_distribution<float> jx(-CW * .5f, CW * .5f);
    std::uniform_real_distribution<float> jy(-RH * .5f, RH * .5f);
    std::uniform_int_distribution<int> dR(3, 10), dG(3, 12), dB(8, 26);
    std::uniform_int_distribution<int> mR(10, 28), mG(8, 22), mB(18, 48);
    std::uniform_int_distribution<int> lR(18, 40), lG(14, 32), lB(28, 60);
    std::uniform_real_distribution<float> shade(0, 1);

    std::vector<sf::Vector2f> pts((COLS + 1) * (ROWS + 1));
    for (int r = 0; r <= ROWS; ++r)
        for (int c = 0; c <= COLS; ++c) {
            bool b = (r == 0 || r == ROWS || c == 0 || c == COLS);
            pts[r * (COLS + 1) + c] = { c * CW + (b ? 0.f : jx(rng())),
                                        r * RH + (b ? 0.f : jy(rng())) };
        }

    m_bgShards.clear();
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c) {
            int tl = r * (COLS + 1) + c, tr = tl + 1;
            int bl = (r + 1) * (COLS + 1) + c, br = bl + 1;
            float s = shade(rng());
            sf::Color col = s < .55f ? sf::Color((uint8_t)dR(rng()), (uint8_t)dG(rng()), (uint8_t)dB(rng()), 255)
                : s < .88f ? sf::Color((uint8_t)mR(rng()), (uint8_t)mG(rng()), (uint8_t)mB(rng()), 255)
                : sf::Color((uint8_t)lR(rng()), (uint8_t)lG(rng()), (uint8_t)lB(rng()), 255);
            for (int v : { tl, tr, bl, tr, br, bl }) m_bgShards.append({ pts[v], col });
        }

    m_bgCracks.clear();
    m_crackPhases.clear();
    std::uniform_real_distribution<float> crack(0, 1);
    std::uniform_real_distribution<float> phaseR(0, 6.283f);
    std::uniform_int_distribution<int> aC(40, 90), rC(70, 140), gC(80, 150), bC(140, 220);

    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c) {
            int tl = r * (COLS + 1) + c, tr = tl + 1, bl = (r + 1) * (COLS + 1) + c;
            if (crack(rng()) < .12f) {
                auto g = sf::Color((uint8_t)rC(rng()), (uint8_t)gC(rng()), (uint8_t)bC(rng()), (uint8_t)aC(rng()));
                m_bgCracks.append({ pts[tl], g });
                m_bgCracks.append({ pts[tr], g });
                m_crackPhases.push_back(phaseR(rng()));
            }
            if (crack(rng()) < .12f) {
                auto g = sf::Color((uint8_t)rC(rng()), (uint8_t)gC(rng()), (uint8_t)bC(rng()), (uint8_t)aC(rng()));
                m_bgCracks.append({ pts[tl], g });
                m_bgCracks.append({ pts[bl], g });
                m_crackPhases.push_back(phaseR(rng()));
            }
        }

    m_bgGradient.clear();
    m_bgGradient.append({ {0, 0},       {0, 0, 0, 255} });
    m_bgGradient.append({ {W, 0},       {0, 0, 0, 255} });
    m_bgGradient.append({ {0, H * .25f},{4, 3, 10, 220} });
    m_bgGradient.append({ {W, H * .25f},{4, 3, 10, 220} });
    m_bgGradient.append({ {0, H * .75f},{4, 3, 10, 220} });
    m_bgGradient.append({ {W, H * .75f},{4, 3, 10, 220} });
    m_bgGradient.append({ {0, H},       {0, 0, 0, 255} });
    m_bgGradient.append({ {W, H},       {0, 0, 0, 255} });
}

void TitleScene::buildEnergyGlow()
{
    constexpr int SEG = 64;
    constexpr float R = 520.f;
    sf::Vector2f center(960.f, 420.f);

    m_bgEnergy.clear();
    m_bgEnergy.append({ center, sf::Color(12, 18, 60, 0) });

    for (int i = 0; i <= SEG; ++i) {
        float a = 2.f * 3.14159265f * i / SEG;
        m_bgEnergy.append({ {center.x + std::cos(a) * R, center.y + std::sin(a) * R},
                            sf::Color(0, 0, 0, 0) });
    }
}

void TitleScene::buildFlashQuad()
{
    constexpr float W = 1920.f, H = 1080.f;
    m_bgFlash.clear();
    sf::Color fc(255, 240, 255, 0);
    m_bgFlash.append({ {0, 0}, fc });
    m_bgFlash.append({ {W, 0}, fc });
    m_bgFlash.append({ {0, H}, fc });
    m_bgFlash.append({ {W, 0}, fc });
    m_bgFlash.append({ {W, H}, fc });
    m_bgFlash.append({ {0, H}, fc });
}

void TitleScene::animateCracks(float time)
{
    for (size_t i = 0; i < m_crackPhases.size(); ++i) {
        float breathe = 0.7f + 0.3f * std::sin(time * 1.8f + m_crackPhases[i]);
        if (m_glitchIntensity > 0.f)
            breathe += m_glitchIntensity * 1.5f;

        float rawA = 40.f * breathe + m_glitchIntensity * 100.f;
        if (rawA < 15.f) rawA = 15.f; if (rawA > 180.f) rawA = 180.f;
        uint8_t a = (uint8_t)rawA;
        sf::Color c(100, 120, 200, a);

        m_bgCracks[i * 2].color = c;
        m_bgCracks[i * 2 + 1].color = c;
    }
}

void TitleScene::buildCharTexts(const std::wstring& str, std::vector<CharData>& out,
    float centerY, float minSize, float maxSize)
{
    out.clear();
    std::uniform_real_distribution<float> szRand(minSize, maxSize);
    std::uniform_real_distribution<float> phRand(0.f, 6.283f);

    float totalW = 0.f;
    std::vector<float> widths, sizes;
    for (size_t i = 0; i < str.size(); ++i) {
        float sz = (i == 0) ? maxSize : szRand(rng());
        sizes.push_back(sz);
        sf::Text probe(m_font);
        probe.setString(sf::String(str.substr(i, 1)));
        probe.setCharacterSize((unsigned)sz);
        widths.push_back(probe.getLocalBounds().size.x);
        totalW += widths.back() + 6.f;
    }
    totalW -= 6.f;

    float cx = 960.f - totalW * .5f;
    for (size_t i = 0; i < str.size(); ++i) {
        CharData cd(m_font);
        cd.text.setString(sf::String(str.substr(i, 1)));
        cd.baseSize = sizes[i];
        cd.sizeVariance = (maxSize / sizes[i] - 1.f) * .3f;
        cd.animPhase = phRand(rng());
        cd.text.setCharacterSize((unsigned)sizes[i]);
        float w = widths[i];
        cd.posX = cx + w * .5f;
        cd.posY = centerY;
        cd.text.setOrigin({ w * .5f, sizes[i] * .4f });
        cd.text.setPosition({ cd.posX, cd.posY });
        cd.text.setFillColor({ 220, 230, 255, 0 });
        cd.text.setOutlineThickness(2.f);
        cd.text.setOutlineColor({ 80, 160, 230, 0 });
        out.push_back(std::move(cd));
        cx += w + 6.f;
    }
}

void TitleScene::triggerGlitch()
{
    m_glitchDuration = .08f + (rng()() % 24) * .01f;
    m_glitchIntensity = .5f + (rng()() % 50) * .01f;
}
