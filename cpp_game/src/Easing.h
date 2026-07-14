#pragma once

#include <cmath>

/// easeOutExpo
inline float easeOutExpo(float t) {
    return (t >= 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
}

/// easeOutBack
inline float easeOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

/// easeOutCubic
inline float easeOutCubic(float t) {
    return 1.0f - std::pow(1.0f - t, 3.0f);
}

/// easeInOutQuad
inline float easeInOutQuad(float t) {
    return (t < 0.5f) ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}
