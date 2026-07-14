#include "BeatmapParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <ctime>
#include <unordered_map>

bool BeatmapParser::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::stringstream buf;
    buf << file.rdbuf();
    std::string json = buf.str();

    m_songInfo.title     = extractString(json, "title");
    m_songInfo.artist    = extractString(json, "artist");
    m_songInfo.musicFile = extractString(json, "musicFile");
    m_songInfo.background = extractString(json, "background");
    m_songInfo.bpm       = extractFloat(json, "bpm");
    m_songInfo.offset    = extractFloat(json, "offset");
    m_songInfo.trackCount = extractInt(json, "trackCount");

    extractNotes(json);
    extractAnomalies(json);

    m_loaded = !m_notes.empty();
    return true;
}

void BeatmapParser::generateExampleBeatmap(int trackCount, float durationSec) {
    m_songInfo = SongInfo{};
    m_songInfo.title = "Example Song";
    m_songInfo.artist = "Demo Artist";
    m_songInfo.bpm = 120.0f;
    m_songInfo.trackCount = trackCount;

    m_notes.clear();
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> trackDist(0, trackCount - 1);
    std::uniform_real_distribution<float> timeDist(1.0f, durationSec);

    float t = 1.0f;
    while (t < durationSec) {
        NoteData note;
        note.time = t;
        note.track = trackDist(rng);
        note.type = 0;
        m_notes.push_back(note);
        t += 0.5f + timeDist(rng) * 0.3f;
    }
    m_loaded = true;
}

// ---- manual JSON helpers (minimal, for our subset only) ----

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

static std::string skipString(const std::string& json, size_t& i) {
    // json[i] is the opening quote
    i++; // skip opening quote
    size_t start = i;
    while (i < json.size() && !(json[i] == '"' && json[i-1] != '\\')) i++;
    std::string val = json.substr(start, i - start);
    if (i < json.size()) i++; // skip closing quote
    return val;
}

static void skipValue(const std::string& json, size_t& i) {
    if (i >= json.size()) return;
    if (json[i] == '"') { skipString(json, i); return; }
    if (json[i] == '[') {
        i++; // skip '['
        int depth = 1;
        while (i < json.size() && depth > 0) {
            if (json[i] == '[') depth++;
            else if (json[i] == ']') depth--;
            else if (json[i] == '"') skipString(json, i);
            if (depth > 0 && i < json.size()) i++;
        }
        if (i < json.size()) i++; // skip ']'
        return;
    }
    if (json[i] == '{') {
        i++; int depth = 1;
        while (i < json.size() && depth > 0) {
            if (json[i] == '{') depth++;
            else if (json[i] == '}') depth--;
            else if (json[i] == '"') skipString(json, i);
            if (depth > 0 && i < json.size()) i++;
        }
        if (i < json.size()) i++;
        return;
    }
    // number / keyword
    while (i < json.size() && json[i] != ',' && json[i] != '}' && json[i] != ']' && json[i] != '\n') i++;
}

std::string BeatmapParser::extractString(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    size_t colon = json.find(':', pos + search.size());
    if (colon == std::string::npos) return "";
    size_t start = json.find_first_of('"', colon);
    if (start == std::string::npos) return "";
    return skipString(json, start);
}

float BeatmapParser::extractFloat(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return 0.0f;
    size_t colon = json.find(':', pos + search.size());
    if (colon == std::string::npos) return 0.0f;
    size_t start = json.find_first_not_of(" \t\r\n", colon + 1);
    if (start == std::string::npos || json[start] == '"') return 0.0f;
    size_t end = start;
    while (end < json.size() && (std::isdigit(json[end]) || json[end] == '.' || json[end] == '-' || json[end] == '+')) end++;
    try { return std::stof(json.substr(start, end - start)); } catch (...) { return 0.0f; }
}

int BeatmapParser::extractInt(const std::string& json, const std::string& key) {
    return static_cast<int>(extractFloat(json, key));
}

void BeatmapParser::extractNotes(const std::string& json) {
    m_notes.clear();
    size_t notesPos = json.find("\"notes\"");

    // no notes key found, try reading as pure note list
    if (notesPos == std::string::npos) return;
    size_t arrStart = json.find('[', notesPos);
    if (arrStart == std::string::npos) return;

    size_t i = arrStart + 1; // skip '['
    while (i < json.size()) {
        // find next '{'
        i = json.find('{', i);
        if (i == std::string::npos || i > json.find(']', arrStart)) break;

        NoteData note;
        // extract from the object at i
        size_t objEnd = json.find('}', i);
        if (objEnd == std::string::npos) break;
        std::string obj = json.substr(i, objEnd - i + 1);

       note.time  = extractFloat(obj, "time");
       note.track = extractInt(obj, "track");
       note.type  = extractInt(obj, "type");
        note.holdDuration = extractFloat(obj, "holdDuration");

        m_notes.push_back(note);
        i = objEnd + 1;
    }
}

// ---- 异象解析 ----

AnomalyType BeatmapParser::anomalyTypeFromString(const std::string& s) {
    static const std::unordered_map<std::string, AnomalyType> map = {
        {"ScreenShake", AnomalyType::ScreenShake},
        {"NoteSpeedChange", AnomalyType::NoteSpeedChange},
        {"LaneShift", AnomalyType::LaneShift},
        {"ColorInvert", AnomalyType::ColorInvert},
        {"ChromaticRift", AnomalyType::ChromaticRift},
        {"Flash", AnomalyType::Flash},
        {"NoteFreeze", AnomalyType::NoteFreeze},
        {"Reverse", AnomalyType::Reverse},
        {"JudgementLineSplit", AnomalyType::JudgementLineSplit},
        {"PerspectiveShift", AnomalyType::PerspectiveShift},
    };
    auto it = map.find(s);
    return (it != map.end()) ? it->second : AnomalyType::Flash;
}

void BeatmapParser::extractAnomalies(const std::string& json) {
    m_anomalies.clear();
    size_t anomPos = json.find("\"anomalies\"");
    if (anomPos == std::string::npos) return;

    size_t arrStart = json.find('[', anomPos);
    if (arrStart == std::string::npos) return;

    // 已知参数键列表（将为每个异象对象尝试提取）
    static const char* paramKeys[] = {"intensity", "speed", "color_r", "color_g", "color_b", "mode"};
    static const int paramCount = 6;

    size_t i = arrStart + 1;
    while (i < json.size()) {
        i = json.find('{', i);
        if (i == std::string::npos || i > json.find(']', arrStart)) break;

        size_t objEnd = json.find('}', i);
        if (objEnd == std::string::npos) break;
        std::string obj = json.substr(i, objEnd - i + 1);

        AnomalyEvent evt;
        evt.triggerTime = extractFloat(obj, "triggerTime");
        evt.duration    = extractFloat(obj, "duration");
        std::string typeStr = extractString(obj, "type");
        evt.type = anomalyTypeFromString(typeStr);

        // 提取额外参数
        for (int k = 0; k < paramCount; ++k) {
            // 搜索参数键（仅当它在对象中存在时）
            std::string search = "\"" + std::string(paramKeys[k]) + "\"";
            if (obj.find(search) != std::string::npos) {
                float val = extractFloat(obj, paramKeys[k]);
                evt.params[paramKeys[k]] = val;
            }
        }

        m_anomalies.push_back(evt);
        i = objEnd + 1;
    }
}
