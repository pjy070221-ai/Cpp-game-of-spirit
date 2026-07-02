#pragma once

#include "Note.h"
#include <vector>
#include <string>

// ╔══════════════════════════════════════════════════════════╗
// ║  ChartLoader — 谱面文件加载器                              ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】从 JSON 文件加载谱面数据，解析为 Chart 结构。       ║
// ║    Chart 包含元信息（BPM、偏移、难度等）和音符数组。         ║
// ║                                                          ║
// ║  【设计特点】                                              ║
// ║    - 静态方法，无需实例化                                   ║
// ║    - 当前 parseFromString() 返回硬编码测试数据              ║
// ║      （待集成 nlohmann/json.hpp 后实现真实解析）            ║
// ║    - 谱面格式参考：assets/charts/boss_song.json            ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    - GameplayScene ：调用 loadFromFile() 获取 Chart        ║
// ║    - Note ：Chart.notes 是 Note 的 vector                  ║
// ║                                                          ║
// ║  【TODO】集成 nlohmann/json 单头文件实现真实解析             ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝

struct ChartMeta
{
    std::string title;
    std::string artist;
    std::string audioFile;
    float       bpm        = 120.0f;
    float       offset     = 0.0f;       // 音频偏移 (ms)
    int         difficulty = 1;
};

struct Chart
{
    ChartMeta meta;
    std::vector<Note> notes;
};

class ChartLoader
{
public:
    static Chart loadFromFile(const std::string& path);
    static Chart parseFromString(const std::string& json);
};
