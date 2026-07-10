#pragma once

#include <vector>
#include <unordered_map>
#include <string>

enum class AnomalyType {
    ScreenShake, NoteSpeedChange, LaneShift, ColorInvert,
    ChromaticRift, Flash, NoteFreeze, Reverse,
    JudgementLineSplit, PerspectiveShift
};

struct AnomalyEvent {
    float triggerTime = 0.0f;
    float duration = 0.0f;
    AnomalyType type = AnomalyType::Flash;
    std::unordered_map<std::string, float> params;
};

class AnomalySystem {
public:
    AnomalySystem() = default;

    void setEvents(const std::vector<AnomalyEvent>& events);
    void update(float songTimeSec, float dt);
    bool isActive(AnomalyType type) const;
    float getIntensity(AnomalyType type) const;
    float getParam(const std::string& key, float defaultVal) const;
    void reset();

private:
    std::vector<AnomalyEvent> m_events;
    int m_nextIndex = 0;
    std::vector<size_t> m_activeIndices;
    float m_currentTime = 0.0f;
};
