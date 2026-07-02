#pragma once

#include <vector>
#include <string>
#include <unordered_map>

// 前向声明
struct Note;

// ╔══════════════════════════════════════════════════════════╗
// ║  AnomalySystem — 异象演出时间轴                            ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】管理谱面中标记的全屏视觉特效。"异象"是音游术语，     ║
// ║    指超出常规玩法的演出效果（如 Arcaea 的异象章节）。        ║
// ║                                                          ║
// ║  【设计特点】                                              ║
// ║    1. 时间轴驱动 — 所有异象事件按 triggerTime 排序，        ║
// ║       update() 中根据歌曲位置触发。                        ║
// ║    2. 活跃列表 — m_activeIndices 记录当前正在播放的效果，    ║
// ║       超时的效果自动移出。                                 ║
// ║    3. 可叠加 — 多个异象可以同时活跃（如闪白+色散+震动）。    ║
// ║    4. 参数化 — 每个事件带 params map，可以传递任意浮点参数   ║
// ║       （如 {"intensity": 0.8, "angle": 180.0}）。          ║
// ║                                                          ║
// ║  【当前支持的异象类型】                                      ║
// ║    ScreenShake / NoteSpeedChange / LaneShift / ColorInvert ║
// ║    ChromaticRift / Flash / NoteFreeze / Reverse           ║
// ║    JudgementLineSplit / PerspectiveShift                  ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    - Note ：从 NoteType::Anomaly 类型的音符提取异象事件     ║
// ║    - GameplayScene ：持有 AnomalySystem 实例，              ║
// ║      每帧 update() → 查询活跃效果 → 传入渲染管线             ║
// ║    - TitleScene 的故障抽搐是独立的简化异象（不依赖此类）     ║
// ║                                                          ║
// ║  【用法】                                                  ║
// ║    anomalySystem.setEvents(myManualEvents);               ║
// ║    anomalySystem.update(songPosMs, dt);                   ║
// ║    if (anomalySystem.isActive(AnomalyType::Flash))        ║
// ║        drawFlashOverlay(anomalySystem.getParam("intensity"));║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝

enum class AnomalyType
{
    ScreenShake,
    NoteSpeedChange,
    LaneShift,
    ColorInvert,
    ChromaticRift,
    Flash,
    NoteFreeze,
    Reverse,
    JudgementLineSplit,
    PerspectiveShift,
};

struct AnomalyEvent
{
    float triggerTime = 0.0f;                                // 触发时间 (ms)
    AnomalyType type  = AnomalyType::Flash;
    float duration    = 500.0f;                               // 持续时间 (ms)
    std::unordered_map<std::string, float> params;           // 可扩展参数

    // 运行时状态
    bool triggered = false;
    bool finished  = false;
};

class AnomalySystem
{
public:
    // 从谱面的 Anomaly 类型 Note 中提取异象事件
    void loadFromNotes(const std::vector<Note>& notes);

    // 手动编排异象时间轴
    void setEvents(const std::vector<AnomalyEvent>& events);

    // 每帧更新：检查触发 + 清理过期事件
    void update(float songPositionMs, float dt);

    // 查询指定类型的异象是否正在播放
    bool isActive(AnomalyType type) const;

    // 获取指定类型的当前强度（多事件叠加取最大值）
    float getIntensity(AnomalyType type) const;

    // 获取参数值（遍历活跃事件查找匹配的 key）
    float getParam(const std::string& key, float defaultValue = 0.0f) const;

    void reset();

private:
    std::vector<AnomalyEvent> m_events;      // 所有异象事件（按时间排序）
    std::vector<size_t>       m_activeIndices; // 当前活跃的事件索引
    int                       m_nextIndex = 0;  // 下一个待触发的事件索引
};

// 字符串 → 异象类型（用于 JSON 谱面解析）
AnomalyType anomalyTypeFromString(const std::string& str);
