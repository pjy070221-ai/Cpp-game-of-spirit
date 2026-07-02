#include "AnomalySystem.h"
#include "../gameplay/Note.h"
#include <algorithm>

AnomalyType anomalyTypeFromString(const std::string& str)
{
    if (str == "screen_shake")       return AnomalyType::ScreenShake;
    if (str == "note_speed")         return AnomalyType::NoteSpeedChange;
    if (str == "lane_shift")         return AnomalyType::LaneShift;
    if (str == "invert")             return AnomalyType::ColorInvert;
    if (str == "chromatic_rift")     return AnomalyType::ChromaticRift;
    if (str == "flash")              return AnomalyType::Flash;
    if (str == "note_freeze")        return AnomalyType::NoteFreeze;
    if (str == "reverse")            return AnomalyType::Reverse;
    if (str == "line_split")         return AnomalyType::JudgementLineSplit;
    if (str == "perspective_shift")  return AnomalyType::PerspectiveShift;
    return AnomalyType::Flash; // 默认
}

void AnomalySystem::loadFromNotes(const std::vector<Note>& notes)
{
    m_events.clear();
    for (const auto& note : notes)
    {
        if (note.type != NoteType::Anomaly)
            continue;

        AnomalyEvent event;
        event.triggerTime = note.time;
        // 从 note 的 endTime 推断 duration
        event.duration = note.endTime > 0.0f ? (note.endTime - note.time) : 500.0f;
        m_events.push_back(event);
    }

    // 按时间排序
    std::sort(m_events.begin(), m_events.end(),
        [](const AnomalyEvent& a, const AnomalyEvent& b) {
            return a.triggerTime < b.triggerTime;
        });
}

void AnomalySystem::setEvents(const std::vector<AnomalyEvent>& events)
{
    m_events = events;
    std::sort(m_events.begin(), m_events.end(),
        [](const AnomalyEvent& a, const AnomalyEvent& b) {
            return a.triggerTime < b.triggerTime;
        });
}

void AnomalySystem::update(float songPositionMs, float dt)
{
    // 检查新触发的事件
    while (m_nextIndex < m_events.size() &&
           m_events[m_nextIndex].triggerTime <= songPositionMs)
    {
        m_events[m_nextIndex].triggered = true;
        m_activeIndices.push_back(m_nextIndex);
        m_nextIndex++;
    }

    // 更新活跃事件，清除已完成的
    m_activeIndices.erase(
        std::remove_if(m_activeIndices.begin(), m_activeIndices.end(),
            [&](size_t idx) {
                auto& evt = m_events[idx];
                float elapsed = songPositionMs - evt.triggerTime;
                if (elapsed >= evt.duration)
                {
                    evt.finished = true;
                    return true;
                }
                return false;
            }),
        m_activeIndices.end()
    );
}

bool AnomalySystem::isActive(AnomalyType type) const
{
    for (size_t idx : m_activeIndices)
    {
        if (m_events[idx].type == type)
            return true;
    }
    return false;
}

float AnomalySystem::getIntensity(AnomalyType type) const
{
    float maxIntensity = 0.0f;
    for (size_t idx : m_activeIndices)
    {
        if (m_events[idx].type == type)
        {
            // 线性插值：从 1.0 衰减到 0
            float elapsed = m_events[idx].triggerTime; // 需要 songPosition
            // 简化：只要活跃就返回 1.0
            maxIntensity = std::max(maxIntensity, 1.0f);
        }
    }
    return maxIntensity;
}

float AnomalySystem::getParam(const std::string& key, float defaultValue) const
{
    // 遍历活跃事件，查找匹配的参数
    for (size_t idx : m_activeIndices)
    {
        auto it = m_events[idx].params.find(key);
        if (it != m_events[idx].params.end())
            return it->second;
    }
    return defaultValue;
}

void AnomalySystem::reset()
{
    m_events.clear();
    m_activeIndices.clear();
    m_nextIndex = 0;
}
