#pragma once

#include "IScene.h"
#include "../core/ParticleSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ╔══════════════════════════════════════════════════════════╗
// ║  TitleScene — 标题画面（目前最完整的场景）                  ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】游戏的第一个画面，展示标题"Abyssal Beat"及入场动画  ║
// ║                                                          ║
// ║  【入场动画状态机】                                        ║
// ║   Loading(2.5s) → Shattering(0.8s) → TitleReveal → Idle  ║
// ║   - Loading   : 纯黑背景 + 进度条 + 星空粒子               ║
// ║   - Shattering: 裂纹从中心蔓延 + 白闪 + 碎裂背景淡入       ║
// ║   - TitleReveal: 标题飞入 + 副标题淡入（复用原入场动画）    ║
// ║   - Idle      : 常态效果（故障、呼吸、粒子）               ║
// ║                                                          ║
// ║  【分层渲染架构】（Idle 态，从底到顶，8 层）                 ║
// ║   Layer 0 — 碎裂背景（静态三角网格，构造一次，之后只 draw） ║
// ║   Layer 1 — 暗角渐变（上下黑角，中央微亮）                 ║
// ║   Layer 2 — 中心能量脉动（TriangleFan，只改中心顶点色/帧） ║
// ║   Layer 3 — 裂纹呼吸（Lines，原地更新顶点 alpha）          ║
// ║   Layer 4 — 浮尘粒子（大而慢，模拟深渊尘埃）               ║
// ║   Layer 5 — 锐利星空（小而亮，加法混合）                   ║
// ║   Layer 6 — 标题文字（预渲染到 RenderTexture → RGB 分裂）  ║
// ║   Layer 7 — 故障闪白（全屏 Quad，仅故障时出现）            ║
// ║   Layer 8 — 提示文字 "Press ENTER"（直接绘制，无特效）     ║
// ║                                                          ║
// ║  【性能优化策略】                                          ║
// ║    - 背景几何体：构造时生成一次，运行时只 draw 不修改        ║
// ║    - 裂纹动画：原地改写顶点颜色（~400 内存写/帧），无分配    ║
// ║    - 能量脉动：只改 1 个顶点颜色/帧                         ║
// ║    - 闪白：只改 4 个顶点 alpha/帧                          ║
// ║    - 文字：合并 16 个单字 → RenderTexture → 3 次全图绘制    ║
// ║      （而非 48 次独立字体绘制）                             ║
// ║                                                          ║
// ║  【CharData 设计】                                         ║
// ║    标题 "Abyssal Beat" 被拆成 12 个独立 sf::Text，每个：    ║
// ║    - 字号随机不均匀（60~90，首字母最大）                    ║
// ║    - 独立上下浮动（正弦波，各不同相）                       ║
// ║    - 字号微变（正弦波，±15%）                              ║
// ║    - 故障时各字独立高频振动                                 ║
// ║    副标题 "Demo" 同理（4 字，幅度减半）                     ║
// ║                                                          ║
// ║  【故障抽搐（Glitch Spike）】（仅 Idle 态）                  ║
// ║    - 每 2~5 秒随机触发，持续 0.08~0.27 秒                  ║
// ║    - 触发时：文字高频振动 + 裂纹变亮 + 全屏闪白             ║
// ║                                                          ║
// ║  【入场动画时间轴】（TitleReveal 态，复用原逻辑）            ║
// ║    0.0~1.5s  — 标题从下方飞入（easeOutExpo + easeOutBack）║
// ║    1.0~2.5s  — 副标题淡入（easeOutCubic）                  ║
// ║    2.5~3.3s  — 提示文字淡入                               ║
// ║    3.3s+     — 提示文字呼吸闪烁                            ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝
class TitleScene : public IScene
{
public:
    TitleScene();
    ~TitleScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    // ═══════════════════════════════════════════════
    // 入场动画状态机
    // ═══════════════════════════════════════════════
    enum class State { Loading, Shattering, TitleReveal, Idle };
    State m_state = State::Loading;

    // Loading 阶段 — 进度条
    float m_loadProgress = 0.f;       // 显示用（可能被 easing 处理过）
    float m_loadRaw = 0.f;            // 原始线性进度，不受 easing 反馈影响
    static constexpr float LOAD_DURATION = 2.5f;

    sf::RectangleShape m_loadTrack;    // 轨道（深灰底色）
    sf::RectangleShape m_loadFill;     // 填充条（蓝青渐变）
    sf::VertexArray m_loadGlow{ sf::PrimitiveType::Triangles }; // 填充条发光
    sf::Text m_loadLabel;             // "LOADING..."
    sf::Text m_loadPercent;           // "72%"

    // Shattering 阶段 — 裂纹爆发
    float m_shatterTimer = 0.f;
    static constexpr float SHATTER_DURATION = 0.8f;
    float m_shardAlpha = 0.f;          // 碎裂背景 → 粒子淡入 (0→1)
    float m_shatterFlashAlpha = 0.f;   // 白闪 alpha
    sf::Vector2f m_impactPoint{ 960.f, 480.f }; // 裂纹爆发中心（屏幕中偏上）

    struct ShatterCrack {
        float angle;     // 裂纹方向（弧度）
        float maxLen;    // 最大长度（像素）
        float delay;     // 0~1 归一化延迟，越大越晚出现
    };
    std::vector<ShatterCrack> m_shatterCrackData;
    sf::VertexArray m_shatterCrackGeom{ sf::PrimitiveType::Lines };

    // 标题动画基准时间（Shattering 结束 → TitleReveal 开始时记录）
    float m_titleStartTime = 0.f;

    // ═══════════════════════════════════════════════
    // 背景图层（构造时生成，运行时按状态选择性绘制）
    // ═══════════════════════════════════════════════

    // ── Layer 0-1: 背景静态几何（构造时生成，运行时只 draw 不改） ──
    sf::VertexArray m_bgShards{ sf::PrimitiveType::Triangles };
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };

    // ── Layer 3: 裂纹（顶点颜色每帧更新实现呼吸脉动） ──
    sf::VertexArray m_bgCracks{ sf::PrimitiveType::Lines };
    std::vector<float> m_crackPhases;       // 每条裂纹独立正弦相位

    // ── Layer 2: 中心能量（TriangleFan 径向渐变，只改中心色） ──
    sf::VertexArray m_bgEnergy{ sf::PrimitiveType::TriangleFan };

    // ── Layer 7: 故障闪白（全屏 2 三角，只改 alpha） ──
    sf::VertexArray m_bgFlash{ sf::PrimitiveType::Triangles };
    float m_flashAlpha = 0.0f;

    // ── Layer 4-5: 粒子系统 ──
    ParticleSystem m_stars;   // 锐利星空背景
    ParticleSystem m_dust;    // 碎片间浮尘

    // ── Layer 6: 文字预渲染纹理（1920×1080 离屏 RenderTexture） ──
    sf::RenderTexture m_textLayer;

    // 单字数据结构 — 每个字独立动画
    struct CharData {
        sf::Text text;                      // SFML 3.1: 需要显式传 font 构造
        float baseSize = 80.f;          // 基准字号（每个字不同）
        float sizeVariance = 0.f;           // 字号微变幅度
        float animPhase = 0.f;           // 独立浮动相位（随机生成）
        float posX = 0.f;           // 基准 X 坐标（用于故障偏移）
        float posY = 0.f;           // 基准 Y 坐标

        CharData(const sf::Font& font) : text(font) {}
    };
    std::vector<CharData> m_titleChars;  // "Abyssal Beat"（12 字）
    std::vector<CharData> m_subChars;    // "Demo"（4 字）

    sf::Font m_font;
    bool m_fontLoaded = false;

    // ── 故障抽搐状态机（仅 Idle 态） ──
    float m_glitchTimer = 0.f;  // 距下一次触发倒计时
    float m_glitchDuration = 0.f;  // 当前抽搐剩余时间
    float m_glitchIntensity = 0.f; // 当前抽搐强度 [0~1]
    void triggerGlitch();            // 随机触发一次抽搐

    // ── 入场动画参数（TitleReveal 态复用） ──
    float m_elapsedTime = 0.f;
    float m_globalAlpha = 0.f;  // 标题整体透明度
    float m_subAlpha = 0.f;  // 副标题透明度
    float m_promptAlpha = 0.f;  // 提示文字透明度
    float m_promptBlinkTimer = 0.f;
    float m_titleTargetY = 380.f; // 标题最终 Y 位置
    float m_titleStartY = 500.f; // 标题起始 Y 位置（下方飞入起点）

    // Layer 8: 提示文字（直接绘制，无特效以保证可读性）
    sf::Text m_promptText;

    // ═══════════════════════════════════════════════
    // 一次性构建函数（仅在构造函数中调用）
    // ═══════════════════════════════════════════════
    void buildShatteredBackground();
    void buildEnergyGlow();
    void buildFlashQuad();
    void buildCharTexts(const std::wstring& str, std::vector<CharData>& out,
        float cy, float minSz, float maxSz);
    void buildShatterCracks();
    void buildLoadingBar();

    // ── 每帧动画 ──
    void animateCracks(float time);

    // ═══════════════════════════════════════════════
    // 状态机 update / render 分发
    // ═══════════════════════════════════════════════
    void updateLoading(float dt);
    void updateShattering(float dt);
    void updateTitleReveal(float dt);
    void updateIdle(float dt);

    void renderLoading(sf::RenderTarget& target);
    void renderShattering(sf::RenderTarget& target);
    void renderNormal(sf::RenderTarget& target);
};
