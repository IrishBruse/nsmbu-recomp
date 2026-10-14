

#pragma once
#include <functional>
#include <string>

namespace hostui {

void post(std::function<void()> fn);

bool get(const char* key, std::string& value);
void set(const char* key, const std::string& value);

float res_scale();
void set_res_scale(float s);
void graphics_changed();
int scale_filter();
void set_scale_filter(int f);
bool scale_filter_available();

bool fullscreen();
void set_fullscreen(bool on);
bool run_in_background();
void set_run_in_background(bool on);
bool app_focused();
void set_app_focused(bool on);
int drc_modes();

bool drc_mode_offered(int m);
int drc_mode();
void set_drc_mode(int m);
int pip_corner();
void set_pip_corner(int c);
float pip_size();
void set_pip_size(float s);
float pip_opacity();
void set_pip_opacity(float o);
bool drc_available();
bool drc_shown();
void show_drc(bool on);
void set_pro_controller(bool on);

const char* name();
void set_clipboard(const std::string& text);
bool can_open_folder();
void open_folder(const std::string& path);

void run_posted();
void load_saved_options();
void toggle_drc();
void drc_window_closed();
void tv_fullscreen_changed();
bool window_size(int& width, int& height);
void window_resized();

}
