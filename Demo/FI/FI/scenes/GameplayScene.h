#pragma once
#include "IScene.h"
#include "../core/ParticleSystem.h"
#include "../gameplay/Note.h"
#include "../gameplay/ChartLoader.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ╔══════════════════════════════════════════════════════════╗
// ║  GameplayScene — "Cross Beat" 十字交叉式演奏              ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【核心理念】音符从四个方向沿十字臂飞向中心判定点。         ║
// ║    十字本身在缓慢旋转，视觉上音符从四面八方汇聚而来。       ║
// ║                                                          ║
// ║  【数学基础】（高中数学即可理解）                          ║
// ║    - 十字有 4 个臂，角度由 θ 控制，臂之间差 90°           ║
// ║    - 每个音符沿着它所属的臂方向飞行                        ║
// ║    - 音符位置 = 中心 + 方向向量 × 到中心的距离             ║
// ║    - 距离递减到 0 时命中                                  ║
// ║                                                          ║
// ║  【优势】                                                  ║
// ║    - 视觉独特，不像传统 4K 下落                            ║
// ║    - 核心只用 sin/cos，大一完全能理解                      ║
// ║    - 代码量与传统模式相当                                  ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝
class GameplayScene : public IScene
{
public:
    GameplayScene();
    ~GameplayScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    // ═══════════════════════════════════════════════
    // 谱面 & 时间
    // ═══════════════════════════════════════════════
    Chart m_chart;
    float m_songTime = 0.f;          // 歌曲时间 (ms)
    size_t m_nextIndex = 0;
    bool m_songFinished = false;

    static constexpr float SPAWN_DIST = 650.f;   // 音符出生距离（距中心像素）
    static constexpr float HIT_DIST = 15.f;      // 命中判定距离
    static constexpr float FALL_DURATION = 2000.f; // 音符飞行时长 (ms)

    // ═══════════════════════════════════════════════
    // 十字旋转系统
    // ═══════════════════════════════════════════════
    float m_crossAngle = 0.f;         // 十字当前旋转角 (rad)
    static constexpr float CROSS_SPEED = 0.35f; // 旋转速度 (rad/s)
    static constexpr float CX = 960.f, CY = 500.f; // 十字中心

    // 计算第 arm 臂（0=右,1=下,2=左,3=上）当前方向向量
    sf::Vector2f armDir(int arm) const {
        float a = m_crossAngle + arm * 1.5707963f; // + arm * π/2
        return { std::cos(a), std::sin(a) };
    }

    // ═══════════════════════════════════════════════
    // 音符运行时数据（在 Chart 的 Note 上扩展）
    // ═══════════════════════════════════════════════
    struct NoteRuntime {
        int  arm = 0;        // 所属臂 (0-3)
        float distance = SPAWN_DIST; // 当前距中心距离
        bool hit = false;
        bool active = false;
        bool holding = false; // Hold 持续中
    };
    std::vector<NoteRuntime> m_noteRuntimes;

    // Flick 独立系统（不沿臂飞行，在臂间生成弧线箭头）
    struct FlickVisual {
        int  fromArm = 0;
        int  toArm = 1;
        bool cw = true;
        float life = 0.f;
        static constexpr float MAX_LIFE = 0.9f;
    };
    std::vector<FlickVisual> m_flicks;

    // ═══════════════════════════════════════════════
    // 分数 & 连击
    // ═══════════════════════════════════════════════
    int m_score = 0;
    int m_combo = 0;
    int m_maxCombo = 0;

    // ═══════════════════════════════════════════════
    // 判定弹出文字
    // ═══════════════════════════════════════════════
    struct JudgePop {
        sf::Text text;
        float life = 0.f;
        static constexpr float MAX_LIFE = 0.55f;
        JudgePop(const sf::Font& f) : text(f) {}
    };
    std::vector<JudgePop> m_judgements;
    void spawnJudge(const std::string& label, const sf::Color& c);

    // ═══════════════════════════════════════════════
    // 粒子
    // ═══════════════════════════════════════════════
    ParticleSystem m_hitFX;
    ParticleSystem m_stars;

    // ═══════════════════════════════════════════════
    // 视觉
    // ═══════════════════════════════════════════════
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    sf::RectangleShape m_noteShape; // 复用模板

    // ═══════════════════════════════════════════════
    // UI
    // ═══════════════════════════════════════════════
    sf::Font m_font;
    sf::Text m_titleText;
    sf::Text m_scoreText;
    sf::Text m_comboText;
    sf::Text m_hintText;

    // ═══════════════════════════════════════════════
    // 方法
    // ═══════════════════════════════════════════════
    void buildBackground();
    void updateNotes();
    void updateJudgements(float dt);
    sf::Color noteColor() const;          // 统一主色
    sf::Color flickColorCW() const;       // 顺时针 Flick
    sf::Color flickColorCCW() const;      // 逆时针 Flick
};
