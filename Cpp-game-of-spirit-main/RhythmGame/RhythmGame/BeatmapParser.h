#pragma once

#include <vector>
#include <string>
#include <map>
#include <json.hpp>

using json = nlohmann::json;

// 音符数据
struct NoteData {
    float time;      // 出现时间（秒）
    int track;       // 轨道 (0-3)
    int type;        // 0=普通, 1=长按(暂不支持)

    NoteData() : time(0), track(0), type(0) {}
    NoteData(float t, int tr) : time(t), track(tr), type(0) {}
};

// 歌曲信息
struct SongInfo {
    std::string title;
    std::string artist;
    std::string musicFile;
    float bpm;
    float offset;
    int trackCount;
};

class BeatmapParser {
public:
    BeatmapParser();
    ~BeatmapParser();

    // 加载谱面文件
    bool loadFromFile(const std::string& filePath);

    // 获取数据
    const std::vector<NoteData>& getNotes() const;
    const SongInfo& getSongInfo() const;
    int getTrackCount() const;
    bool isLoaded() const;

    // 获取在指定时间范围内的音符
    std::vector<NoteData> getNotesAtTime(float currentTime, float lookAhead = 2.0f);

    // 生成示例谱面（用于测试）
    void generateExampleBeatmap(int trackCount = 4, float duration = 30.0f);

private:
    std::vector<NoteData> notes;
    SongInfo songInfo;
    bool loaded;
    int noteIndex;

    // 解析 JSON
    bool parseJSON(const json& data);
};
