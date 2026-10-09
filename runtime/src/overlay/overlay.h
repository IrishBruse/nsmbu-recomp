

#pragma once
#include <cstdint>

struct ImDrawData;

namespace overlay {

bool is_open();
void set_open(bool open);

bool blocks_input();

bool captures();

bool alive();
bool perf_shown();
void set_perf_shown(bool on);

enum Mods : int { kShift = 1, kCtrl = 2, kAlt = 4, kSuper = 8 };

bool key(int code, bool down, bool repeat, int mods);

bool mouse_move(float nx, float ny);
bool mouse_button(int button, bool down);
bool mouse_wheel(float dx, float dy);
void set_density(float pixels_per_point);

ImDrawData* frame(float pw, float ph, void (*renderer_init)());

void linearize_colors(ImDrawData* d);

}
