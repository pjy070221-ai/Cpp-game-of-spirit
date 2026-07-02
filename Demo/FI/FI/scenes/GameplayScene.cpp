#include "GameplayScene.h"
#include "PackScene.h"
#include "ResultScene.h"
#include "SettingsScene.h"
#include "../core/Easing.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

static constexpr float PI = 3.14159265f;

// ═══════════════════════════════════════════════
// 构造 & 生命周期
// ═══════════════════════════════════════════════

GameplayScene::GameplayScene()
    : m_titleText(m_font)
    , m_scoreText(m_font)
    , m_comboText(m_font)
    , m_hintText(m_font)
{
    m_font.openFromFile("C:/Windows/Fonts/msyh.ttc") ||
        m_font.openFromFile("C:/Windows/Fonts/arial.ttf");

    buildBackground();

    // UI
    m_titleText.setCharacterSize(20);
    m_titleText.setFillColor({ 140, 160, 200, 200 });
    m_titleText.setPosition({ 40.f, 30.f });

    m_scoreText.setCharacterSize(22);
    m_scoreText.setFillColor({ 200, 210, 240, 220 });
    m_scoreText.setPosition({ 1700.f, 30.f });

    m_comboText.setCharacterSize(44);
    m_comboText.setFillColor({ 255, 240, 200, 220 });

    m_hintText.setString("ESC  Back");
    m_hintText.setCharacterSize(16);
    m_hintText.setFillColor({ 100, 110, 140, 120 });
    m_hintText.setPosition({ 40.f, 1040.f });
}

void GameplayScene::onEnter()
{
    m_songTime = 0.f;
    m_nextIndex = 0;
    m_score = 0;
    m_combo = 0;
    m_maxCombo = 0;
    m_songFinished = false;
    m_crossAngle = 0.f;

    m_chart = ChartLoader::parseFromString("");
    m_noteRuntimes.clear();
    m_noteRuntimes.resize(m_chart.notes.size());
    int flickIdx = 0;
    for (size_t i = 0; i < m_chart.notes.size(); ++i)
    {
        if (m_chart.notes[i].type == NoteType::Flick)
            m_noteRuntimes[i].arm = (flickIdx++) % 4; // Flick 独立循环
        else
            m_noteRuntimes[i].arm = (int)i % 4;
    }

    m_judgements.clear();
    m_flicks.clear();
    m_hitFX.clear();
    m_stars.clear();
}

void GameplayScene::onExit()
{
    m_hitFX.clear();
    m_stars.clear();
}

// ═══════════════════════════════════════════════
// 背景
// ═══════════════════════════════════════════════

void GameplayScene::buildBackground()
{
    constexpr float W = 1920.f, H = 1080.f;
    m_bgGradient.clear();
    // 从中心向外的暗角：中心微亮，边缘全黑
    m_bgGradient.append({ {0, 0},       {2, 1, 6, 255} });
    m_bgGradient.append({ {W, 0},       {2, 1, 6, 255} });
    m_bgGradient.append({ {0, H * .35f},{6, 4, 18, 180} });
    m_bgGradient.append({ {W, H * .35f},{6, 4, 18, 180} });
    m_bgGradient.append({ {0, H * .65f},{6, 4, 18, 180} });
    m_bgGradient.append({ {W, H * .65f},{6, 4, 18, 180} });
    m_bgGradient.append({ {0, H},       {2, 1, 6, 255} });
    m_bgGradient.append({ {W, H},       {2, 1, 6, 255} });
}

// ═══════════════════════════════════════════════
// 颜色
// ═══════════════════════════════════════════════

sf::Color GameplayScene::noteColor() const     { return { 180, 200, 240 }; }
sf::Color GameplayScene::flickColorCW() const  { return { 255, 200, 80 }; }
sf::Color GameplayScene::flickColorCCW() const { return { 80, 200, 240 }; }

// ═══════════════════════════════════════════════
// 判定文字
// ═══════════════════════════════════════════════

void GameplayScene::spawnJudge(const std::string& label, const sf::Color& c)
{
    JudgePop jp(m_font);
    jp.text.setString(label);
    jp.text.setCharacterSize(30);
    jp.text.setFillColor(c);
    sf::FloatRect b = jp.text.getLocalBounds();
    jp.text.setOrigin({ b.size.x * .5f, b.size.y * .5f });
    jp.text.setPosition({ CX, CY - 50.f });
    jp.life = JudgePop::MAX_LIFE;
    m_judgements.push_back(std::move(jp));
}

// ═══════════════════════════════════════════════
// 每帧逻辑
// ═══════════════════════════════════════════════

void GameplayScene::update(float dt)
{
    if (m_songFinished) return;

    m_songTime += dt * 1000.f;
    m_crossAngle += CROSS_SPEED * dt;

    // ── 激活新音符（Flick 跳过距离系统） ──
    while (m_nextIndex < m_chart.notes.size())
    {
        auto& n = m_chart.notes[m_nextIndex];
        if (n.time - m_songTime > FALL_DURATION) break;

        if (n.type == NoteType::Flick)
        {
            // Flick 不沿臂飞行 — 到触发时间前等待，别跳过
            if (m_songTime < n.time) break;

            bool cw = (n.lane == 0);
            int arm = m_noteRuntimes[m_nextIndex].arm;
            int toArm = cw ? (arm + 1) % 4 : (arm + 3) % 4;
            FlickVisual fv;
            fv.fromArm = arm;
            fv.toArm = toArm;
            fv.cw = cw;
            fv.life = FlickVisual::MAX_LIFE;
            m_flicks.push_back(fv);
            m_noteRuntimes[m_nextIndex].hit = true;
            m_noteRuntimes[m_nextIndex].active = true;
            m_combo++;
            if (m_combo > m_maxCombo) m_maxCombo = m_combo;
            m_score += 1000 + m_combo * 10;
            spawnJudge("PERFECT", { 255, 220, 80 });
            ++m_nextIndex;
            continue;
        }

        m_noteRuntimes[m_nextIndex].active = true;
        m_noteRuntimes[m_nextIndex].distance = SPAWN_DIST;
        ++m_nextIndex;
    }

    // ── Flick 视觉生命周期 ──
    for (auto& fv : m_flicks) fv.life -= dt;
    m_flicks.erase(std::remove_if(m_flicks.begin(), m_flicks.end(),
        [](const FlickVisual& f) { return f.life <= 0.f; }), m_flicks.end());

    // ── 更新音符距离 + 判定 ──
    updateNotes();

    // ── 判定文字 ──
    updateJudgements(dt);

    // ── 粒子 ──
    m_hitFX.update(dt);
    sf::FloatRect area({ 0, 0 }, { 1920, 1080 });
    m_stars.spawnStars(dt, area, 30);
    m_stars.update(dt);

    // ── 检查结束 → 自动跳转 ResultScene ──
    if (m_nextIndex >= m_chart.notes.size())
    {
        bool allDone = true;
        for (auto& rt : m_noteRuntimes)
            if (rt.active && !rt.hit) { allDone = false; break; }
        if (allDone && m_songTime > m_chart.notes.back().time + 2000.f)
        {
            m_songFinished = true;
            GameResult gr;
            gr.score = m_score;
            gr.maxCombo = m_maxCombo;
            gr.songTitle = m_chart.meta.title;
            requestReplace(std::make_unique<ResultScene>(gr));
            return;
        }
    }

    // ── UI ──
    m_titleText.setString(m_chart.meta.title);
    char sbuf[32];
    snprintf(sbuf, sizeof(sbuf), "Score: %d", m_score);
    m_scoreText.setString(sbuf);

    if (m_combo > 0)
    {
        char cbuf[32];
        snprintf(cbuf, sizeof(cbuf), "%d", m_combo);
        m_comboText.setString(cbuf);
        float pulse = 1.f + (m_combo >= 10 ? 0.06f * std::sin(m_songTime * 0.012f) : 0.f);
        m_comboText.setCharacterSize((unsigned)(44.f * pulse));
        sf::FloatRect cb = m_comboText.getLocalBounds();
        m_comboText.setOrigin({ cb.size.x * .5f, cb.size.y * .5f });
        m_comboText.setPosition({ CX, CY + 100.f });
    }
}

void GameplayScene::updateNotes()
{
    for (size_t i = 0; i < m_chart.notes.size(); ++i)
    {
        auto& rt = m_noteRuntimes[i];
        if (!rt.active) continue;

        auto& note = m_chart.notes[i];
        float remain = note.time - m_songTime;

        // ── Hold 持续期间 ──
        if (rt.holding)
        {
            // 持续粒子喷雾
            if (m_songTime > note.time && m_songTime < note.endTime)
            {
                sf::Vector2f dir = armDir(rt.arm);
                sf::Color lc = noteColor();
                m_hitFX.emit({ CX + dir.x * 10.f, CY + dir.y * 10.f },
                              2, lc, 20.f, 100.f, 0.1f, 0.3f, 0.5f, 2.f);
            }
            // Hold 释放
            if (m_songTime >= note.endTime)
            {
                rt.holding = false;
                // 释放爆发粒子
                sf::Vector2f dir = armDir(rt.arm);
                sf::Color lc = noteColor();
                m_hitFX.emit({ CX, CY }, 20, lc, 50.f, 200.f, 0.1f, 0.4f, 1.f, 4.f);
            }
            continue;
        }

        if (rt.hit) continue;

        // 到中心距离（受流速设置影响）
        float speedMul = GameSettings::inst().noteSpeed;
        rt.distance = (remain / (FALL_DURATION / speedMul)) * SPAWN_DIST;

        // 到达中心 → 判定
        if (remain <= 0.f && rt.distance < HIT_DIST)
        {
            rt.hit = true;
            m_combo++;
            if (m_combo > m_maxCombo) m_maxCombo = m_combo;
            m_score += 1000 + m_combo * 10;

            sf::Vector2f dir = armDir(rt.arm);
            sf::Color lc = noteColor();

            if (note.type == NoteType::Hold)
            {
                // Hold 开始 — 持续按压
                rt.holding = true;
                m_hitFX.emit({ CX, CY }, 40, lc, 80.f, 300.f, 0.2f, 0.6f, 1.f, 5.f);
                spawnJudge("HOLD", { 100, 220, 255 });
            }
            else
            {
                // Tap 单点
                m_hitFX.emit({ CX - dir.x * 20.f, CY - dir.y * 20.f },
                              30, lc, 60.f, 250.f, 0.15f, 0.5f, 1.f, 5.f);
                spawnJudge("PERFECT", { 255, 220, 80 });
            }
        }
        // 超出判定窗口
        else if (remain < -200.f)
        {
            rt.hit = true;
            m_combo = 0;
            spawnJudge("MISS", { 255, 80, 80 });
        }
    }
}

void GameplayScene::updateJudgements(float dt)
{
    for (auto& j : m_judgements)
    {
        j.life -= dt;
        float t = j.life / JudgePop::MAX_LIFE;
        j.text.setScale({ 1.f + (1.f - t) * 0.7f, 1.f + (1.f - t) * 0.7f });
        auto c = j.text.getFillColor();
        c.a = (uint8_t)(t * 255.f);
        j.text.setFillColor(c);
        j.text.move({ 0.f, -50.f * dt });
    }
    m_judgements.erase(
        std::remove_if(m_judgements.begin(), m_judgements.end(),
            [](const JudgePop& j) { return j.life <= 0.f; }),
        m_judgements.end());
}

// ═══════════════════════════════════════════════
// 渲染
// ═══════════════════════════════════════════════

void GameplayScene::render(sf::RenderTarget& target)
{
    // ── 背景 ──
    target.draw(m_bgGradient);
    m_stars.render(target);

    // ══ 旋转十字 ══
    // 4 条臂，每条臂 = 中心向外发散的线
    for (int a = 0; a < 4; ++a)
    {
        sf::Vector2f d = armDir(a);
        sf::Color lc = noteColor();

        // 臂主干（粗线）
        sf::VertexArray arm{ sf::PrimitiveType::Lines };
        sf::Color armC(lc.r, lc.g, lc.b, 60);
        arm.append({ {CX, CY}, armC });
        arm.append({ {CX + d.x * SPAWN_DIST, CY + d.y * SPAWN_DIST},
                     { lc.r, lc.g, lc.b, 15 } });
        target.draw(arm);

        // 臂发光（稍宽、更透明）
        sf::VertexArray glow{ sf::PrimitiveType::Triangles };
        sf::Vector2f perp(-d.y, d.x); // 垂直方向
        sf::Color gc(lc.r, lc.g, lc.b, 20);
        sf::Color gt(lc.r, lc.g, lc.b, 0);
        float gw = 5.f;
        glow.append({ {CX + perp.x * gw, CY + perp.y * gw}, gt });
        glow.append({ {CX - perp.x * gw, CY - perp.y * gw}, gt });
        glow.append({ {CX + d.x * SPAWN_DIST + perp.x * gw,
                       CY + d.y * SPAWN_DIST + perp.y * gw}, gc });
        glow.append({ {CX - perp.x * gw, CY - perp.y * gw}, gt });
        glow.append({ {CX + d.x * SPAWN_DIST - perp.x * gw,
                       CY + d.y * SPAWN_DIST - perp.y * gw}, gc });
        glow.append({ {CX + d.x * SPAWN_DIST + perp.x * gw,
                       CY + d.y * SPAWN_DIST + perp.y * gw}, gc });
        target.draw(glow);
    }

    // ══ 中心判定环 ══
    {
        float pulse = 1.f + 0.06f * std::sin(m_songTime * 0.005f);
        float ringR = 30.f * pulse;
        sf::CircleShape ring(ringR);
        ring.setOrigin({ ringR, ringR });
        ring.setPosition({ CX, CY });
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(2.f);
        ring.setOutlineColor({ 160, 200, 255, 180 });
        target.draw(ring);

        // 内点
        sf::CircleShape dot(6.f);
        dot.setOrigin({ 6.f, 6.f });
        dot.setPosition({ CX, CY });
        dot.setFillColor({ 200, 230, 255, 220 });
        target.draw(dot);
    }

    // ══ 音符 ══
    for (size_t i = 0; i < m_chart.notes.size(); ++i)
    {
        auto& rt = m_noteRuntimes[i];
        if (!rt.active || rt.hit) continue;
        if (rt.distance > SPAWN_DIST + 50.f) continue;

        sf::Vector2f d = armDir(rt.arm);
        float nx = CX + d.x * rt.distance;
        float ny = CY + d.y * rt.distance;
        float closeness = 1.f - rt.distance / SPAWN_DIST;
        float sz = 10.f + closeness * 4.f;

        auto& note = m_chart.notes[i];

        if (note.type == NoteType::Flick) continue; // Flick 由独立系统渲染
        if (note.type == NoteType::Hold)
        {
            // ── Hold: 胶囊形（两端半圆 + 中间矩形） ──
            sf::Color hc = noteColor();
            sf::Color hg((uint8_t)(hc.r + (255 - hc.r) * closeness * .5f),
                         (uint8_t)(hc.g + (255 - hc.g) * closeness * .5f),
                         (uint8_t)(hc.b + (255 - hc.b) * closeness * .5f), 255);
            float hw = sz * 1.5f;  // 半宽
            float hl = sz * 2.5f;  // 半长
            sf::Vector2f side(-d.y * hw, d.x * hw);
            sf::Vector2f along(d.x * hl, d.y * hl);

            // 矩形主体
            sf::VertexArray body{ sf::PrimitiveType::Triangles };
            body.append({ {nx + side.x - along.x, ny + side.y - along.y}, hg });
            body.append({ {nx - side.x - along.x, ny - side.y - along.y}, hg });
            body.append({ {nx + side.x + along.x, ny + side.y + along.y}, hg });
            body.append({ {nx - side.x - along.x, ny - side.y - along.y}, hg });
            body.append({ {nx - side.x + along.x, ny - side.y + along.y}, hg });
            body.append({ {nx + side.x + along.x, ny + side.y + along.y}, hg });
            target.draw(body);

            // 两端半圆（简化为小三角扇形）
            for (int end = -1; end <= 1; end += 2)
            {
                float cx2 = nx + along.x * (float)end;
                float cy2 = ny + along.y * (float)end;
                sf::VertexArray cap{ sf::PrimitiveType::TriangleFan };
                cap.append({ {cx2, cy2}, hc });
                int segs = 6;
                for (int s = 0; s <= segs; ++s)
                {
                    float a = PI * s / segs;
                    float rx = -side.x * std::cos(a) + along.x * (float)end * std::sin(a) * 0.4f;
                    float ry = -side.y * std::cos(a) + along.y * (float)end * std::sin(a) * 0.4f;
                    cap.append({ {cx2 + rx, cy2 + ry}, hg });
                }
                target.draw(cap);
            }
        }
        else
        {
            // ── Tap: 菱形 ──
            sf::Color nc = noteColor();
            sf::Color ng((uint8_t)(nc.r + (255 - nc.r) * closeness * .6f),
                         (uint8_t)(nc.g + (255 - nc.g) * closeness * .6f),
                         (uint8_t)(nc.b + (255 - nc.b) * closeness * .6f), 255);

            // Chord 检测: 同一时刻有其他音符 → 加外圈高光
            bool isChord = false;
            for (size_t j = 0; j < m_chart.notes.size(); ++j)
            {
                if (i == j) continue;
                if (std::abs(m_chart.notes[j].time - note.time) < 10.f &&
                    m_chart.notes[j].type == NoteType::Tap &&
                    m_noteRuntimes[j].arm != rt.arm)
                { isChord = true; break; }
            }

            sf::Vector2f side(-d.y * sz, d.x * sz);
            // 菱形主体
            sf::VertexArray diamond{ sf::PrimitiveType::TriangleStrip };
            diamond.append({ {nx - side.x, ny - side.y}, ng });
            diamond.append({ {nx - d.x * sz * 1.5f, ny - d.y * sz * 1.5f}, nc });
            diamond.append({ {nx + side.x, ny + side.y}, ng });
            diamond.append({ {nx + d.x * sz * 1.5f, ny + d.y * sz * 1.5f}, nc });
            diamond.append({ {nx - side.x, ny - side.y}, ng });
            target.draw(diamond);

            // Chord 外圈高光
            if (isChord)
            {
                sf::CircleShape halo(sz * 2.2f);
                halo.setOrigin({ sz * 2.2f, sz * 2.2f });
                halo.setPosition({ nx, ny });
                halo.setFillColor(sf::Color::Transparent);
                halo.setOutlineThickness(1.5f);
                halo.setOutlineColor({ 255, 255, 255, (uint8_t)(closeness * 180.f) });
                target.draw(halo);
            }
        }
    }

    // ══ Hold 光柱 ══
    for (size_t i = 0; i < m_chart.notes.size(); ++i)
    {
        auto& rt = m_noteRuntimes[i];
        if (!rt.holding) continue;

        sf::Vector2f d = armDir(rt.arm);
        sf::Color lc = noteColor();
        float holdLen = SPAWN_DIST;
        sf::VertexArray beam{ sf::PrimitiveType::Triangles };
        sf::Vector2f perp(-d.y, d.x);
        float bw = 6.f;
        sf::Color bc(lc.r, lc.g, lc.b, 100);
        sf::Color bt(lc.r, lc.g, lc.b, 0);

        beam.append({ {CX + perp.x * bw, CY + perp.y * bw}, bt });
        beam.append({ {CX - perp.x * bw, CY - perp.y * bw}, bt });
        beam.append({ {CX + d.x * holdLen + perp.x * bw,
                       CY + d.y * holdLen + perp.y * bw}, bc });
        beam.append({ {CX - perp.x * bw, CY - perp.y * bw}, bt });
        beam.append({ {CX + d.x * holdLen - perp.x * bw,
                       CY + d.y * holdLen - perp.y * bw}, bc });
        beam.append({ {CX + d.x * holdLen + perp.x * bw,
                       CY + d.y * holdLen + perp.y * bw}, bc });
        target.draw(beam);
    }

    // ══ Flick 弧线箭头（maimai 式动态流动） ══
    for (auto& fv : m_flicks)
    {
        sf::Vector2f fromD = armDir(fv.fromArm);
        sf::Vector2f toD = armDir(fv.toArm);
        float midR = SPAWN_DIST * 0.55f;
        sf::Vector2f fromPt{ CX + fromD.x * midR, CY + fromD.y * midR };
        sf::Vector2f toPt{ CX + toD.x * midR, CY + toD.y * midR };

        sf::Color fc = fv.cw ? flickColorCW() : flickColorCCW();
        float t = fv.life / FlickVisual::MAX_LIFE;
        uint8_t alpha = (uint8_t)(t * 200.f);

        float aFrom = std::atan2(fromPt.y - CY, fromPt.x - CX);
        float aTo = std::atan2(toPt.y - CY, toPt.x - CX);
        if (fv.cw && aTo < aFrom) aTo += 2.f * PI;
        if (!fv.cw && aTo > aFrom) aTo -= 2.f * PI;

        // 静态底弧（暗色导轨）
        {
            sf::VertexArray guide{ sf::PrimitiveType::TriangleStrip };
            for (int i = 0; i <= 16; ++i) {
                float a = aFrom + (aTo - aFrom) * i / 16.f;
                sf::Vector2f pt{ CX + std::cos(a) * midR, CY + std::sin(a) * midR };
                sf::Vector2f p{ -std::sin(a) * 3.f, std::cos(a) * 3.f };
                sf::Color g(fc.r, fc.g, fc.b, alpha / 6);
                guide.append({ {pt.x + p.x, pt.y + p.y}, g });
                guide.append({ {pt.x - p.x, pt.y - p.y}, g });
            }
            target.draw(guide);
        }

        // 4 个流动光点沿弧线循环跑
        float flowSpeed = 1.8f; // 周期 / 秒
        for (int d = 0; d < 4; ++d)
        {
            float phase = std::fmod(m_songTime * 0.001f * flowSpeed + d * 0.25f, 1.f);
            float a = aFrom + (aTo - aFrom) * phase;
            sf::Vector2f pt{ CX + std::cos(a) * midR, CY + std::sin(a) * midR };
            sf::Vector2f p{ -std::sin(a) * 10.f, std::cos(a) * 10.f };

            sf::VertexArray dash{ sf::PrimitiveType::Triangles };
            // 光点前后延伸 ~15°
            float da = (aTo - aFrom) * 0.04f;
            sf::Vector2f p2{ -std::sin(a + da) * 4.f, std::cos(a + da) * 4.f };
            sf::Vector2f n2{ -std::sin(a - da) * 4.f, -std::cos(a - da) * 4.f };

            sf::Color bright(fc.r, fc.g, fc.b, alpha);
            sf::Color dim(fc.r, fc.g, fc.b, alpha / 8);

            dash.append({ {pt.x + p.x, pt.y + p.y}, bright });
            dash.append({ {pt.x - p.x, pt.y - p.y}, bright });
            dash.append({ {pt.x + p2.x * (float)std::cos(da), pt.y + p2.y * (float)std::cos(da)}, dim });
            dash.append({ {pt.x - p.x, pt.y - p.y}, bright });
            dash.append({ {pt.x + n2.x * (float)std::cos(da), pt.y + n2.y * (float)std::cos(da)}, dim });
            dash.append({ {pt.x + p.x, pt.y + p.y}, bright });
            target.draw(dash);
        }

        // 箭头头
        sf::Vector2f tipDir{ std::cos(aTo), std::sin(aTo) };
        sf::Vector2f headBase{ toPt.x, toPt.y };
        sf::Vector2f headTip{ toPt.x + tipDir.x * 40.f, toPt.y + tipDir.y * 40.f };
        sf::Vector2f headSide{ -tipDir.y * 35.f, tipDir.x * 35.f };
        sf::VertexArray head{ sf::PrimitiveType::Triangles };
        sf::Color hc(fc.r, fc.g, fc.b, alpha);
        sf::Color he(fc.r, fc.g, fc.b, alpha / 3);
        head.append({ {headBase.x + headSide.x, headBase.y + headSide.y}, hc });
        head.append({ {headBase.x - headSide.x, headBase.y - headSide.y}, hc });
        head.append({ {headTip.x, headTip.y}, he });
        target.draw(head);
    }

    // ══ 命中粒子 ══
    m_hitFX.render(target);

    // ══ 判定文字 ══
    for (auto& j : m_judgements)
        target.draw(j.text);

    // ══ UI ══
    target.draw(m_titleText);
    target.draw(m_scoreText);
    if (m_combo > 0)
        target.draw(m_comboText);

    // ══ 结束 ══
    if (m_songFinished)
    {
        sf::Text done(m_font, "SONG COMPLETE", 40);
        done.setFillColor({ 255, 220, 100, 200 });
        sf::FloatRect db = done.getLocalBounds();
        done.setOrigin({ db.size.x * .5f, db.size.y * .5f });
        done.setPosition({ 960.f, 540.f });
        target.draw(done);

        char mbuf[64];
        snprintf(mbuf, sizeof(mbuf), "Score: %d    Max Combo: %d", m_score, m_maxCombo);
        sf::Text info(m_font, mbuf, 22);
        info.setFillColor({ 200, 200, 220, 180 });
        sf::FloatRect ib = info.getLocalBounds();
        info.setOrigin({ ib.size.x * .5f, ib.size.y * .5f });
        info.setPosition({ 960.f, 600.f });
        target.draw(info);
    }

    target.draw(m_hintText);
}

// ═══════════════════════════════════════════════
// 输入
// ═══════════════════════════════════════════════

void GameplayScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Escape)
            requestReplace(std::make_unique<PackScene>());
    }
}
