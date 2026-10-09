

#pragma once
#include <cstdint>

namespace interp {
void record_executed_step();
uint64_t executed_steps();
int mode();
void set_mode(int m);
int fps();
void set_fps(int f);
void toggle_fps(int f);
const char* mode_name();
int output_fps();
int frames_per_step();

bool paced_interpolation();
void set_paced_interpolation(bool on);
bool paced_interpolation_at(int fps);
void set_paced_interpolation_at(int fps, bool on);
float paced_drawn_share();

void set_display_hz(int hz);
int display_hz();
void set_present_vsync(bool on);
}
