#pragma once

// ╔══════════════════════════════════════════════════════════╗
// ║  Note — 音符数据结构                                      ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】谱面中一个音符的完整描述。包括类型、轨道、时间、    ║
// ║    运行时渲染状态。                                       ║
// ║                                                          ║
// ║  【字段说明】                                              ║
// ║    type    — Tap(点)/Hold(长按)/Flick(滑)/Anomaly(异象)  ║
// ║    lane    — 轨道编号 1~N（1=最左）                        ║
// ║    time    — 触发时刻 (ms)                                ║
// ║    endTime — Hold 音符结束时刻 (ms)，其他类型为 0          ║
// ║    y       — 运行时屏幕 Y 坐标（GameplayScene 计算）       ║
// ║    hit     — 是否已被判定（自动演奏中总是 true）            ║
// ║    NoteType::Anomaly 类型的音符不是可击打音符，             ║
// ║    而是触发 AnomalySystem 的事件标记                       ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    - ChartLoader ：生成 Note 数组                         ║
// ║    - GameplayScene ：在 update() 中更新 y，在 render() 中  ║
// ║      根据 type 绘制不同样式的音符                          ║
// ║    - AutoPlayer ：根据 Note 序列生成按键事件               ║
// ║    - AnomalySystem ：提取 type==Anomaly 的 Note            ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝

enum class NoteType
{
    Tap,      // 单击 — 普通蓝白色矩形
    Hold,     // 长按 — 金色矩形 + 尾部延长线
    Flick,    // 滑动 — 暂未实现渲染
    Anomaly   // 异象标记 — 不渲染，仅触发 AnomalySystem 事件
};

struct Note
{
    NoteType type    = NoteType::Tap;
    int      lane    = 0;           // 轨道编号 1~max
    float    time    = 0.0f;        // 触发时刻 (ms)
    float    endTime = 0.0f;        // Hold 结束时刻 (ms)

    // ── 运行时渲染状态（由 GameplayScene 维护） ──
    float    y       = 0.0f;        // 当前屏幕 Y 坐标
    bool     hit     = false;       // 已被自动判定
    bool     active  = true;        // 是否在活跃中
};
