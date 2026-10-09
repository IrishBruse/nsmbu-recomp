

#pragma once
#include <cstdint>
#include <cstring>

namespace audio {

struct Fader {
    float value = 0, target = 0;
    uint32_t frames = 0;
    float goal() const { return frames ? target : value; }
};

inline bool gamepad_audible(const Fader& tv, const Fader& drc) {
    if (drc.value <= 0 && drc.goal() <= 0) return false;
    return !(tv.value >= 1 && tv.goal() >= 1);
}

class OutputSelect {
public:
    enum Source { kAuto, kTv, kGamePad };
    Source source = kAuto;

    static Source parse(const char* s) {
        if (!s) return kAuto;
        if (!strcmp(s, "tv")) return kTv;
        if (!strcmp(s, "gamepad")) return kGamePad;
        return kAuto;
    }

    void update(bool faders_ok, const Fader& tv, const Fader& drc) {
        play_tv = source != kGamePad;
        play_drc = source == kGamePad || (source == kAuto && faders_ok && gamepad_audible(tv, drc));
    }
    bool play_tv = true, play_drc = false;

    void mix(const int32_t* tv_l, const int32_t* tv_r, const int32_t* drc_l, const int32_t* drc_r, int n, int32_t* out_l,
             int32_t* out_r) const {
        for (int i = 0; i < n; i++) {
            out_l[i] = (play_tv ? tv_l[i] : 0) + (play_drc ? drc_l[i] : 0);
            out_r[i] = (play_tv ? tv_r[i] : 0) + (play_drc ? drc_r[i] : 0);
        }
    }
};

}
