
#pragma once
#include <cstdint>

namespace aspect {
enum Mode : int { kOriginal = 0, kWindow, k16x10, k21x9, k32x9, kCustom, kModes };
int mode();
void set_mode(int m);
const char* mode_name(int m);
float mode_value(int m);
void set_window_aspect(float a);
float requested();

float on_swap();
float game();
uint64_t game_frame();

bool tagged_projection();
void layout_root_target(uint32_t root, bool tv);
void set_content_clip(bool clip);
bool content_clip();
void ss_reset();
}
