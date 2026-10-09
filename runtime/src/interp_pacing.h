

#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace interp {
namespace pacing {

inline float lerp_f(float a, float b, float t) { return t == 0.5f ? 0.5f * (a + b) : (1.0f - t) * a + t * b; }

inline int16_t lerp_s16(int16_t a, int16_t b, float t) {
    const int16_t d = (int16_t)(b - a);
    return t == 0.5f ? (int16_t)(a + d / 2) : (int16_t)(a + (int)std::lround(d * t));
}
inline uint8_t lerp_u8(uint8_t a, uint8_t b, float t) {
    return t == 0.5f ? (uint8_t)((a + b + 1) / 2) : (uint8_t)std::lround(a + (b - a) * t);
}

inline void slerp_weights(float cosang, float t, float& wa, float& wb) {
    if (t == 0.5f) {
        wa = wb = 1.0f;
        return;
    }
    const float ang = std::acos(std::min(1.0f, std::max(-1.0f, cosang)));
    if (ang > 1e-4f) {
        const float s = std::sin(ang);
        wa = std::sin((1 - t) * ang) / s;
        wb = std::sin(t * ang) / s;
    } else {
        wa = 1 - t;
        wb = t;
    }
}

inline float pass_fraction(int phase, int n, bool exact) {
    if (phase == 0) return exact ? 1.0f : 1.0f / float(n + 1);
    return phase < n ? float(phase + 1) / float(n + 1) : 1.0f;
}

inline int valid_fps(int f) { return f >= 240 ? 240 : f >= 165 ? 165 : f >= 120 ? 120 : 60; }

inline int cap_fps(int fps, int hz, bool vsync) {
    fps = valid_fps(fps);
    if (!vsync || hz <= 0) return fps;
    const int shown = hz >= 238 ? 240 : hz >= 163 ? 165 : hz >= 118 ? 120 : 60;
    return std::min(fps, shown);
}

inline int plan_in_between(int n_max, int64_t budget, int64_t logic_pass, int64_t hold_pass) {
    n_max = std::max(1, n_max);
    hold_pass = std::max<int64_t>(hold_pass, 1);
    const int64_t fit = (budget - std::max<int64_t>(logic_pass, 0)) / hold_pass;
    return int(std::clamp<int64_t>(fit, 1, n_max));
}

inline int64_t update_average(int64_t avg, int64_t sample) {
    if (avg > 0) sample = std::min(sample, 2 * avg);
    return (avg * 3 + sample) / 4;
}

enum class Next {
    kStepDone,
    kHold,
    kRecord,
    kDrop,
};
inline Next next_pass(int phase, int n, int64_t elapsed, int64_t pass, int64_t budget) {
    if (phase >= n) return Next::kStepDone;
    const int remaining = n - phase;
    if (elapsed + pass * remaining <= budget) return Next::kHold;

    if (remaining > 1 && elapsed + pass <= budget) return Next::kRecord;
    return Next::kDrop;
}

}
}
