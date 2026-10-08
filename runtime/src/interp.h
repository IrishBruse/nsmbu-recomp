// Frame rate modes (interp.cpp): the API the menus, the settings overlay, the hosts and the
// renderers use.
#pragma once
#include <cstdint>

namespace interp {
int mode();                 // 0 is 60 fps, 1 draws at fps()
void set_mode(int m);
int fps();                  // 60, 120, 165 or 240
void set_fps(int f);
void toggle_fps(int f);     // that rate on, or 60 fps if it is on already
const char* mode_name();    // "60 fps", "120 fps", "165 fps", "240 fps"; "240 fps (165 shown)" when capped
int output_fps();           // the rate frame interpolation draws: fps() capped to the display
int frames_per_step();      // frames drawn per 60 fps step: 1, 2, 3 or 4

// "Keep game speed": in-between frames that do not fit are skipped instead of slowing the game down;
// one setting for 60 fps and one for 120/240 fps
bool paced_interpolation();  // at the current rate
void set_paced_interpolation(bool on);
bool paced_interpolation_at(int fps);
void set_paced_interpolation_at(int fps, bool on);
float paced_drawn_share();  // share of in-between frames drawn lately (paced and on), -1 otherwise

// the display (hosts and renderers): refresh rate of the TV window's screen (0: unknown), and
// whether presenting waits for its vsync (Metal, Vulkan FIFO)
void set_display_hz(int hz);
int display_hz();           // (NSMBU_DISPLAY_HZ overrides the detected rate)
void set_present_vsync(bool on);
}  // namespace interp
