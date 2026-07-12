#include "AnomalySystem.h"
#include <algorithm>
#include <cmath>

void AnomalySystem::setEvents(const std::vector<AnomalyEvent>& events) {
    m_events = events;
    m_nextIndex = 0;
    m_activeIndices.clear();
}

void AnomalySystem::update(float songTimeSec, float dt) {
    m_currentTime = songTimeSec;

    while (m_nextIndex < (int)m_events.size() &&
           m_events[m_nextIndex].triggerTime <= m_currentTime) {
        if (m_events[m_nextIndex].triggerTime + m_events[m_nextIndex].duration >= m_currentTime) {
            m_activeIndices.push_back(m_nextIndex);
        }
        m_nextIndex++;
    }

    auto dead = std::remove_if(m_activeIndices.begin(), m_activeIndices.end(),
        [&](size_t idx) {
            return m_currentTime >= m_events[idx].triggerTime + m_events[idx].duration;
        });
    m_activeIndices.erase(dead, m_activeIndices.end());
}

bool AnomalySystem::isActive(AnomalyType type) const {
    for (size_t idx : m_activeIndices)
        if (m_events[idx].type == type) return true;
    return false;
}

float AnomalySystem::getIntensity(AnomalyType type) const {
    for (size_t idx : m_activeIndices) {
        const auto& e = m_events[idx];
        if (e.type != type) continue;
        float elapsed = e.duration > 0.0f ? (m_currentTime - e.triggerTime) / e.duration : 1.0f;
        float t = std::clamp(elapsed, 0.0f, 1.0f);
        return (t < 0.5f) ? t * 2.0f : (1.0f - t) * 2.0f;
    }
    return 0.0f;
}

float AnomalySystem::getParam(const std::string& key, float defaultVal) const {
    for (size_t idx : m_activeIndices) {
        auto it = m_events[idx].params.find(key);
        if (it != m_events[idx].params.end()) return it->second;
    }
    return defaultVal;
}

void AnomalySystem::reset() {
    m_events.clear();
    m_nextIndex = 0;
    m_activeIndices.clear();
}
