#include "ChartLoader.h"
#include <fstream>
#include <sstream>

Chart ChartLoader::loadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return {};
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseFromString(buffer.str());
}

// ═══════════════════════════════════════════════
// 多样键型谱面生成器
// ═══════════════════════════════════════════════

static void addTap(Chart& c, float t, int arm) {
    Note n; n.type = NoteType::Tap; n.lane = arm; n.time = t; c.notes.push_back(n);
}
static void addHold(Chart& c, float t, float et, int arm) {
    Note n; n.type = NoteType::Hold; n.lane = arm; n.time = t; n.endTime = et; c.notes.push_back(n);
}
// Flick: arm 为飞行方向，lane 编码旋转方向 (0=CW顺时针, 1=CCW逆时针)
static void addFlick(Chart& c, float t, int arm, bool cw) {
    Note n; n.type = NoteType::Flick; n.lane = cw ? 0 : 1; n.time = t; c.notes.push_back(n);
}
// Chord: 两臂同时 Tap（靠判定时检测同时间不同臂来加高光）
static void addChord(Chart& c, float t, int a1, int a2) {
    addTap(c, t, a1); addTap(c, t, a2);
}

Chart ChartLoader::parseFromString(const std::string& /*json*/)
{
    Chart chart;
    chart.meta.title = "Cross Beat Demo";
    chart.meta.bpm   = 160.0f;
    chart.meta.offset = 0.0f;

    float t = 2000.f; // 起始时间 (ms)

    // ── Intro 留白 2s（纯视觉） ──

    // ═══ Stair × 2（0→1→2→3 旋转楼梯）═════
    // 第一圈
    for (int i = 0; i < 4; ++i) { addTap(chart, t, i);       t += 150.f; }
    // 第二圈（更密集）
    for (int i = 0; i < 4; ++i) { addTap(chart, t, i);       t += 130.f; }
    t += 100.f; // 段落间隙

    // ═══ Trill 0↔1 + 背景 Single on 3 ═════
    for (int i = 0; i < 12; ++i) {
        addTap(chart, t, i % 2);     // 0↔1 快速交替
        if (i % 3 == 0) addTap(chart, t, 3); // arm 3 偶发单点
        t += 120.f;
    }
    t += 100.f;

    // ═══ Chord 连击（双押带高光）═════
    for (int i = 0; i < 4; ++i) {
        addChord(chart, t, 0, 2); // 左右 Chord
        t += 350.f;
        addChord(chart, t, 1, 3); // 上下 Chord
        t += 350.f;
    }

    // ═══ Flick 交替 ═════
    for (int i = 0; i < 8; ++i) {
        addFlick(chart, t, i % 4, i % 2 == 0); // CW/CCW 交替
        t += 180.f;
    }
    t += 100.f;

    // ═══ Jump 连发（对立臂对撞）═════
    for (int i = 0; i < 4; ++i) {
        addTap(chart, t, 0); addTap(chart, t, 2);
        t += 350.f;
        addTap(chart, t, 1); addTap(chart, t, 3);
        t += 350.f;
    }

    // ═══ FullCross 高潮 ═════
    t += 100.f;
    for (int a = 0; a < 4; ++a) addTap(chart, t, a); // 4 臂齐发
    t += 400.f;

    // ═══ Stair 反向（3→2→1→0）═════
    for (int i = 3; i >= 0; --i) { addTap(chart, t, i); t += 140.f; }
    for (int i = 3; i >= 0; --i) { addTap(chart, t, i); t += 120.f; }
    t += 100.f;

    // ═══ Hold arm 0 + Trill 1↔3 ═════
    addHold(chart, t, t + 1500.f, 0); // arm 0 长按 1.5s
    for (int i = 0; i < 12; ++i) {
        addTap(chart, t + i * 125.f, (i % 2) ? 3 : 1); // 1↔3 乒乓
    }
    t += 1600.f;

    // ═══ Jump + Single 混合 ═════
    addTap(chart, t, 0); addTap(chart, t, 2); t += 250.f;
    addTap(chart, t, 1); t += 200.f;
    addTap(chart, t, 0); addTap(chart, t, 2); t += 250.f;
    addTap(chart, t, 3); t += 200.f;
    addTap(chart, t, 1); addTap(chart, t, 3); t += 250.f;
    addTap(chart, t, 0); addTap(chart, t, 2); t += 200.f;
    addTap(chart, t, 1); addTap(chart, t, 3); t += 150.f;

    // ═══ 终结 FullCross ═════
    t += 100.f;
    for (int a = 0; a < 4; ++a) addTap(chart, t, a);
    t += 1500.f; // outro

    return chart;
}
