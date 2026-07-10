#pragma once

#include <cmath>

/// easeOutExpo — 指数缓出（TitleScene 标题飞入用）
inline float easeOutExpo(float t) {
    return (t >= 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
}

/// easeOutBack — 回弹缓出
inline float easeOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

/// easeOutCubic — 立方缓出
inline float easeOutCubic(float t) {
    return 1.0f - std::pow(1.0f - t, 3.0f);
}

/// easeInOutQuad — 二次缓入缓出
inline float easeInOutQuad(float t) {
    return (t < 0.5f) ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}
