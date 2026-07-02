#pragma once

#include "Note.h"
#include <vector>

// ╔══════════════════════════════════════════════════════════╗
// ║  AutoPlayer — 自动演奏按键生成器                            ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】根据谱面 Note 序列，生成模拟玩家按键的事件流。       ║
// ║    这是"演示模式"的核心组件 — 替代真人输入。                ║
// ║                                                          ║
// ║  【设计特点】                                              ║
// ║    - 静态方法，无状态                                       ║
// ║    - buildSequence() : 一次性生成完整按键序列                ║
// ║      （press 事件在 note.time，release 在 endTime 或 +50ms）║
// ║    - pollEvents() : 根据时间窗口过滤事件                    ║
// ║    - Anomaly 类型的 Note 不产生按键事件                     ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    - Note ：输入，读取 Note 数组生成 HitEvent               ║
// ║    - GameplayScene ：调用 buildSequence 生成序列，          ║
// ║      在 update() 中调用 pollEvents 获取本帧事件             ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝

struct HitEvent
{
    int   lane;           // 轨道编号
    float time;           // 事件时刻 (ms)
    bool  pressed;        // true=按下, false=释放
};

class AutoPlayer
{
public:
    // 将谱面音符转换为完整按键序列（按时间排序）
    static std::vector<HitEvent> buildSequence(const std::vector<Note>& notes);

    // 轮询 [lastTimeMs, currentTimeMs] 区间内的事件
    static std::vector<HitEvent> pollEvents(
        const std::vector<HitEvent>& sequence,
        float currentTimeMs, float lastTimeMs
    );
};
