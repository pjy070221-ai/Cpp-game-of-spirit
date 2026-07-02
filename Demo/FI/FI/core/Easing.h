#pragma once

#include <cmath>

// ╔══════════════════════════════════════════════════════════╗
// ║  Easing — 缓动函数命名空间                                 ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】提供 10+ 个标准缓动函数，用于 UI 动画、            ║
// ║    标题入场、卡片过渡等。输入 t ∈ [0,1]，输出也在 [0,1]。   ║
// ║                                                          ║
// ║  【设计特点】                                              ║
// ║    - 全部 inline，无编译依赖                               ║
// ║    - 纯数学公式，无状态                                    ║
// ║    - 以 easeIn/Out/InOut 三种变体覆盖常见需求              ║
// ║                                                          ║
// ║  【当前项目中使用的函数】                                   ║
// ║    easeOutExpo    — 标题入场（快→慢，有冲击力）            ║
// ║    easeOutBack    — 标题回弹定位（过冲再回弹）             ║
// ║    easeOutCubic   — 副标题和提示淡入                       ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    被所有 Scene 的 update() 使用，控制动画节奏              ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝

namespace Easing
{
    constexpr float PI = 3.14159265359f;

    // ── 二次方 ──
    inline float easeInQuad(float t)    { return t * t; }
    inline float easeOutQuad(float t)   { return t * (2.0f - t); }
    inline float easeInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }

    // ── 三次方（最常用的平滑过渡） ──
    inline float easeInCubic(float t)   { return t * t * t; }
    inline float easeOutCubic(float t)  { float u = 1.0f - t; return 1.0f - u * u * u; }
    inline float easeInOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
    }

    // ── 指数（快速到达，用于标题冲击） ──
    inline float easeOutExpo(float t) {
        return t >= 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
    }
    inline float easeInOutExpo(float t) {
        if (t <= 0.0f) return 0.0f; if (t >= 1.0f) return 1.0f;
        if (t < 0.5f) return std::pow(2.0f, 20.0f * t - 10.0f) * 0.5f;
        return (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) * 0.5f;
    }

    // ── 弹性（打击反馈感） ──
    inline float easeOutElastic(float t) {
        if (t <= 0.0f || t >= 1.0f) return std::clamp(t, 0.0f, 1.0f);
        return std::pow(2.0f, -10.0f * t) * std::sin((t - 0.075f) * (2.0f * PI) / 0.3f) + 1.0f;
    }

    // ── 回弹（标题定位用 — 过冲后回弹） ──
    inline float easeOutBack(float t) {
        constexpr float c1 = 1.70158f, c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
    }
    inline float easeInOutBack(float t) {
        constexpr float c1 = 1.70158f, c2 = c1 * 1.525f;
        return t < 0.5f
            ? (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) * 0.5f
            : (std::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) * 0.5f;
    }

    // ── 弹跳（UI 元素自然落下） ──
    inline float easeOutBounce(float t) {
        constexpr float n1 = 7.5625f, d1 = 2.75f;
        if (t < 1.0f / d1)       return n1 * t * t;
        else if (t < 2.0f / d1)  { float u = t - 1.5f / d1; return n1 * u * u + 0.75f; }
        else if (t < 2.5f / d1)  { float u = t - 2.25f / d1; return n1 * u * u + 0.9375f; }
        else                     { float u = t - 2.625f / d1; return n1 * u * u + 0.984375f; }
    }

    // ── 平滑步进（无缓入缓出，均匀过渡） ──
    inline float smoothstep(float t) {
        float u = std::clamp(t, 0.0f, 1.0f);
        return u * u * (3.0f - 2.0f * u);
    }
}
