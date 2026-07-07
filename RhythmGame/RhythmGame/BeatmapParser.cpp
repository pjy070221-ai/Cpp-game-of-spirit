#include "BeatmapParser.h"
#include <fstream>
#include <iostream>
#include <random>

BeatmapParser::BeatmapParser()
    : loaded(false)
    , noteIndex(0) {
    songInfo.trackCount = 4;
}

BeatmapParser::~BeatmapParser() {
}

bool BeatmapParser::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "[BeatmapParser] Failed to open: " << filePath << std::endl;
        return false;
    }

    try {
        json data;
        file >> data;
        return parseJSON(data);
    }
    catch (const std::exception& e) {
        std::cout << "[BeatmapParser] Parse error: " << e.what() << std::endl;
        return false;
    }
}

bool BeatmapParser::parseJSON(const json& data) {
    // 解析歌曲信息
    if (data.contains("title")) songInfo.title = data["title"].get<std::string>();
    if (data.contains("artist")) songInfo.artist = data["artist"].get<std::string>();
    if (data.contains("musicFile")) songInfo.musicFile = data["musicFile"].get<std::string>();
    if (data.contains("bpm")) songInfo.bpm = data["bpm"].get<float>();
    if (data.contains("offset")) songInfo.offset = data["offset"].get<float>();
    if (data.contains("trackCount")) songInfo.trackCount = data["trackCount"].get<int>();

    // 解析音符
    if (data.contains("notes")) {
        notes.clear();
        for (const auto& note : data["notes"]) {
            NoteData nd;
            nd.time = note["time"].get<float>();
            nd.track = note["track"].get<int>();
            if (note.contains("type")) nd.type = note["type"].get<int>();
            notes.push_back(nd);
        }
        std::cout << "[BeatmapParser] Loaded " << notes.size() << " notes" << std::endl;
    }

    loaded = true;
    noteIndex = 0;
    return true;
}

const std::vector<NoteData>& BeatmapParser::getNotes() const {
    return notes;
}

const SongInfo& BeatmapParser::getSongInfo() const {
    return songInfo;
}

int BeatmapParser::getTrackCount() const {
    return songInfo.trackCount;
}

bool BeatmapParser::isLoaded() const {
    return loaded;
}

std::vector<NoteData> BeatmapParser::getNotesAtTime(float currentTime, float lookAhead) {
    std::vector<NoteData> result;

    while (noteIndex < notes.size() && notes[noteIndex].time <= currentTime + lookAhead) {
        if (notes[noteIndex].time >= currentTime) {
            result.push_back(notes[noteIndex]);
        }
        noteIndex++;
    }

    return result;
}

void BeatmapParser::generateExampleBeatmap(int trackCount, float duration) {
    notes.clear();
    songInfo.trackCount = trackCount;
    songInfo.title = "Example Song";
    songInfo.artist = "Auto Generated";
    songInfo.musicFile = "";
    songInfo.bpm = 120.0f;
    songInfo.offset = 0.0f;

    // 生成一些示例音符
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> trackDist(0, trackCount - 1);
    std::uniform_real_distribution<> timeDist(0.5f, 2.0f);

    float currentTime = 1.0f;
    while (currentTime < duration) {
        int track = trackDist(gen);
        notes.push_back(NoteData(currentTime, track));

        // 随机间隔 0.3-1.0 秒
        currentTime += 0.3f + (float)(std::rand() % 70) / 100.0f;
    }

    loaded = true;
    noteIndex = 0;
    std::cout << "[BeatmapParser] Generated " << notes.size() << " example notes" << std::endl;
}