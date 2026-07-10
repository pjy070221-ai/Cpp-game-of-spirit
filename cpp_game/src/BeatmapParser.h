#pragma once

#include <string>
#include <vector>
#include "Types.h"

class BeatmapParser {
public:
    BeatmapParser() = default;

    // load chart from a JSON file; returns true on success
    bool loadFromFile(const std::string& filePath);

    // get parsed results
    const std::vector<NoteData>& getNotes() const { return m_notes; }
    const SongInfo& getSongInfo() const { return m_songInfo; }

    int  getTrackCount() const { return m_songInfo.trackCount; }
    bool isLoaded() const { return m_loaded; }

    // generate a random example chart for testing
    void generateExampleBeatmap(int trackCount, float durationSec);

private:
    // simple JSON key-value extraction helpers
    std::string extractString(const std::string& json, const std::string& key);
    float       extractFloat(const std::string& json, const std::string& key);
    int         extractInt(const std::string& json, const std::string& key);
    void        extractNotes(const std::string& json);

    std::vector<NoteData> m_notes;
    SongInfo m_songInfo;
    bool m_loaded = false;
};
