#include "nsmbu_guest.h"

#include "nsmbu/functions.h"

#define SMOOTH_STEP NSMBU_ADDR_cLib_addCalc2

NSMBU_GAME_ORIGINAL(SMOOTH_STEP, void, smooth_step_original, (f32* value, f32 target, f32 scale, f32 max_step));

static u32 calls, own;

NSMBU_REPLACE(SMOOTH_STEP, void, smooth_step, (f32* value, f32 target, f32 scale, f32 max_step)) {
    ++calls;
    if ((calls & 0xFFFF) == 1) {
        nsmbu_log_int("smooth-step-replace: calls", (int)calls);
        nsmbu_log_int("smooth-step-replace: handled by the mod", (int)own);
    }
    if (calls & 1) {
        smooth_step_original(value, target, scale, max_step);
        return;
    }
    ++own;
    f32 v = *value;
    if (v == target) return;
    f32 step = (target - v) * scale;
    if (step > max_step) step = max_step;
    else if (step < -max_step) step = -max_step;
    *value = v + step;
}
