

#pragma once
#include <atomic>
#include <cstdint>

#include "display.h"

namespace gfx {

enum DrcMode { kDrcWindow, kDrcPip, kDrcAuto, kDrcOff, kDrcGamePad, kDrcModeCount };
enum Filter { kSmooth, kSharp, kInteger };
extern const char* const kModeNames[kDrcModeCount];
extern const char* const kCornerNames[4];
extern const char* const kFilterNames[3];

extern std::atomic<int> g_mode;
extern std::atomic<int> g_corner;
extern std::atomic<float> g_pip_size;
extern std::atomic<float> g_pip_opacity;
extern std::atomic<int> g_filter;
extern std::atomic<bool> g_shown;
extern std::atomic<bool> g_auto_pin;
extern std::atomic<double> g_auto_until;
extern std::atomic<float> g_drc_aspect;
extern std::atomic<bool> g_has_drc_window;

int find_name(const char* const* names, int n, const char* s, int def);
double display_now();
bool drc_mode_offered(int m);

void display_env_overrides();

bool display_start_fullscreen(bool saved, bool hidden_windows);
bool display_fullscreen_env();

int display_test_mode(uint64_t frame);

bool drc_window_wanted();
bool pip_shown_now();
bool drc_screen_shown(bool drc_window_visible);
void display_show_drc(bool on);
void display_set_mode(int m);
void display_touched();

void display_plus_pressed();
int display_saved_mode();

struct Layout { Box tv, pip; bool pip_on = false; bool drc_only = false; float scale = 1; };

Layout layout(float dw, float dh, float tw, float th, float pw, float ph, bool pip_on, bool drc_only = false);

bool overlay_hit(float nx, float ny, float* tx, float* ty, bool clamp_outside = false);

bool main_picture(float* x, float* y, float* w, float* h);

bool view_button_enabled();
bool view_button_hit(float nx, float ny);
int next_view();

}
