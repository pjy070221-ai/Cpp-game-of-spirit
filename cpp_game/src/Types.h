#pragma once

#include <string>

enum class JudgeResult {
    Perfect,
    Great,
    Good,
    Miss,
    None
};

struct NoteData {
    float time = 0.0f;
    int   track = 0;
    int   type = 0;          // 0 = normal, 1 = hold
    float holdDuration = 0.0f; // seconds, 0 for normal notes

    NoteData() = default;
    NoteData(float t, int tr) : time(t), track(tr) {}
};

struct SongInfo {
    std::string title;
    std::string artist;
    std::string musicFile;
    std::string background;  // 背景图路径（可选）
    float       bpm = 120.0f;
    float       offset = 0.0f;
    int         trackCount = 4;
};

struct NoteRuntime {
    int   track = 0;
    float targetTime = 0.0f;
    int   type = 0;          // 0 = normal, 1 = hold
    float holdDuration = 0.0f;
    bool  isHeld = false;     // player is holding this hold note

    float y = 0.0f;
    bool  processed = false;
    bool  active = false;
};
