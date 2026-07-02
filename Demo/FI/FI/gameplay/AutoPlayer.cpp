#include "AutoPlayer.h"
#include <algorithm>

std::vector<HitEvent> AutoPlayer::buildSequence(const std::vector<Note>& notes)
{
    std::vector<HitEvent> sequence;

    for (const auto& note : notes)
    {
        if (note.type == NoteType::Anomaly)
            continue; // 异象事件不产生按键

        // 按下事件
        HitEvent press;
        press.lane    = note.lane;
        press.time    = note.time;
        press.pressed = true;
        sequence.push_back(press);

        // Hold 音符的释放事件
        if (note.type == NoteType::Hold)
        {
            HitEvent release;
            release.lane    = note.lane;
            release.time    = note.endTime;
            release.pressed = false;
            sequence.push_back(release);
        }
        else
        {
            // Tap 在极短时间后释放
            HitEvent release;
            release.lane    = note.lane;
            release.time    = note.time + 50.0f; // 50ms 后释放
            release.pressed = false;
            sequence.push_back(release);
        }
    }

    // 按时间排序
    std::sort(sequence.begin(), sequence.end(),
        [](const HitEvent& a, const HitEvent& b) {
            return a.time < b.time;
        });

    return sequence;
}

std::vector<HitEvent> AutoPlayer::pollEvents(
    const std::vector<HitEvent>& sequence,
    float currentTimeMs,
    float lastTimeMs)
{
    std::vector<HitEvent> result;

    // 找出时间窗口 [lastTimeMs, currentTimeMs] 内的事件
    for (const auto& event : sequence)
    {
        if (event.time > lastTimeMs && event.time <= currentTimeMs)
            result.push_back(event);
    }

    return result;
}
