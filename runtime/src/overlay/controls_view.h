

#pragma once
#include "../input_map.h"

namespace overlay {

enum { kColKey0, kColKey1, kColPad, kColAny };

struct ControlsView {

    input_map::Mapping m;
    bool pro = false;
    int cap_action = -1, cap_col = 0;

    const float* pad = nullptr;
    const bool* keys = nullptr;
    float act[input_map::kActionCount] = {};
    float stick[2][2] = {};
    double t = 0;

    int hover_action = -1, hover_col = -1;
    int start_action = -1, start_col = 0;
    int clear_action = -1, clear_col = 0;
};

void draw_controls(ControlsView& v, float w, float h);

}
