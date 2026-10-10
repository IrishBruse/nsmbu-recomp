#include "../renderer.h"

#ifdef NSMBU_SDL_HOST
#include "backend.h"
#include "gfx/display_modes.h"
#include "settings.h"
#include "input.h"
#include "interp.h"
#include "overlay/hostui.h"
#include "platform/host.h"
#include "platform/perf_hint.h"
#include "runtime.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <vector>

namespace hostui {
namespace {
std::mutex g_mu;
std::vector<std::function<void()>> g_posted;
std::map<std::string, std::string> g_values;
bool g_loaded = false;

std::string path() {
    if (const char* e = getenv("NSMBU_SETTINGS")) return e;
    if (getenv("NSMBU_NO_HOST_INPUT")) return {};
    return host::config_dir() + "/settings.ini";
}
void load_locked() {
    if (g_loaded) return;
    g_loaded = true;
    std::string p = path();
    if (p.empty()) return;
    std::ifstream in(p);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq != std::string::npos) g_values[line.substr(0, eq)] = line.substr(eq + 1);
    }
}
void save_locked() {
    std::string p = path();
    if (p.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(p).parent_path(), ec);
    std::string tmp = p + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        out << "# New Super Mario Bros. U settings (settings overlay, F1)\n";
        for (auto& [k, v] : g_values) out << k << '=' << v << '\n';
        if (!out) return;
    }
    if (!host::replace_file(tmp, p)) LOG("[settings] cannot write %s", p.c_str());
}
bool env_set(std::initializer_list<const char*> env) {
    for (const char* e : env)
        if (getenv(e)) return true;
    return false;
}

void save_display_locked() {
    using namespace gfx;
    if (!env_set({"NSMBU_DRC_MODE"})) g_values["drcMode"] = kModeNames[display_saved_mode()];
    if (!env_set({"NSMBU_DRC_PIP"})) {
        char v[16];
        g_values["pipCorner"] = kCornerNames[g_corner];
        snprintf(v, sizeof v, "%g", g_pip_size.load());
        g_values["pipSize"] = v;
        snprintf(v, sizeof v, "%g", g_pip_opacity.load());
        g_values["pipOpacity"] = v;
    }
    save_locked();
}
void save_display() {
    std::lock_guard<std::mutex> lk(g_mu);
    load_locked();
    save_display_locked();
}

void apply_drc_window() {
    SDL_Window* w = gfxvk::R.drc.window;
    if (!w) return;
    const bool want = gfx::drc_window_wanted();
    const bool shown = !(SDL_GetWindowFlags(w) & SDL_WINDOW_HIDDEN);
    if (want && !shown && !getenv("NSMBU_HIDDEN_WINDOWS")) {
        SDL_ShowWindow(w);
        gfxvk::R.drc.visible = true;
    } else if (!want && shown) {
        if (SDL_GetWindowFlags(w) & SDL_WINDOW_FULLSCREEN) SDL_SetWindowFullscreen(w, false);
        SDL_HideWindow(w);
        gfxvk::R.drc.visible = false;
    }
}
}

void post(std::function<void()> fn) {
    std::lock_guard<std::mutex> lk(g_mu);
    g_posted.push_back(std::move(fn));
}
void choose_mod_source(bool folder, std::function<void(std::string)> chosen) {
    post([folder, chosen] {
        auto callback = new std::function<void(std::string)>(chosen);
        auto done = [](void* context, const char* const* files, int) {
            auto fn = static_cast<std::function<void(std::string)>*>(context);
            if (files && files[0]) (*fn)(files[0]);
            delete fn;
        };
        if (folder) SDL_ShowOpenFolderDialog(done, callback, nullptr, nullptr, false);
        else SDL_ShowOpenFileDialog(done, callback, nullptr, nullptr, 0, nullptr, false);
    });
}

void run_posted() {
    std::vector<std::function<void()>> fns;
    {
        std::lock_guard<std::mutex> lk(g_mu);
        fns.swap(g_posted);
    }
    for (auto& f : fns) f();
}

bool get(const char* key, std::string& value) {
    std::lock_guard<std::mutex> lk(g_mu);
    load_locked();
    auto it = g_values.find(key);
    if (it == g_values.end()) return false;
    value = it->second;
    return true;
}
void set(const char* key, const std::string& value) {
    std::lock_guard<std::mutex> lk(g_mu);
    load_locked();
    g_values[key] = value;
    save_locked();
}

float res_scale() { return gfxvk::requested_res_scale(); }
void set_res_scale(float s) { gfxvk::set_res_scale(s); }

void graphics_changed() {
    std::lock_guard<std::mutex> lk(g_mu);
    load_locked();
    auto put = [&](const char* key, std::initializer_list<const char*> env, const std::string& v) {
        if (!env_set(env)) g_values[key] = v;
    };
    char res[16];
    snprintf(res, sizeof res, "%g", gfxvk::requested_res_scale());
    put("resScale", {"NSMBU_RES_SCALE"}, res);
    put("aoMode", {"NSMBU_AO_MODE", "NSMBU_NO_AO_QUIRK"}, std::to_string(gfxvk::ao_mode()));
    put("aoHires", {"NSMBU_AO_HIRES"}, gfxvk::ao_hires_enabled() ? "1" : "0");
    put("aniso", {"NSMBU_ANISO"}, gfxvk::aniso_enabled() ? "1" : "0");
    put("bloomStrength", {"NSMBU_BLOOM_STRENGTH"}, std::to_string(render::bloom_strength()));
    put("fxaa", {"NSMBU_FXAA"}, gfxvk::fxaa_enabled() ? "1" : "0");
#ifdef __ANDROID__

    put("fps60", {"NSMBU_INTERP", "NSMBU_TRUE60", "NSMBU_INTERP_FPS"}, interp::mode() == 2 ? "2" : perf_hint::fps60_chosen() ? "1" : "0");
#else
    put("fps60", {"NSMBU_INTERP", "NSMBU_TRUE60", "NSMBU_INTERP_FPS"}, std::to_string(interp::mode()));
#endif

    put("interpFps", {"NSMBU_INTERP_FPS"}, std::to_string(interp::fps()));
    put("fps60Paced", {"NSMBU_INTERP_PACED"}, interp::paced_interpolation_at(60) ? "1" : "0");
    put("fpsHighPaced", {"NSMBU_INTERP_PACED"}, interp::paced_interpolation_at(120) ? "1" : "0");
    put("scaleFilter", {"NSMBU_SCALE_FILTER"}, std::to_string(gfxvk::scale_filter()));
    if (!env_set({"NSMBU_VK_PRESENT_MODE"}) && gfxvk::present_mode_user_set()) {
        g_values["vkPresentMode"] = std::to_string(gfxvk::present_mode());
        g_values["vkPresentChosen"] = "1";
    }
    save_locked();
}

void load_saved_options() {
    std::map<std::string, std::string> v;
    {
        std::lock_guard<std::mutex> lk(g_mu);
        load_locked();
        v = g_values;
    }
    auto saved = [&](const char* key, std::initializer_list<const char*> env) { return !env_set(env) && v.count(key); };
    auto num = [&](const char* key) { return atof(v[key].c_str()); };
    if (saved("resScale", {"NSMBU_RES_SCALE"})) gfxvk::set_res_scale((float)num("resScale"));
    if (saved("aoMode", {"NSMBU_AO_MODE", "NSMBU_NO_AO_QUIRK"})) gfxvk::set_ao_mode((int)num("aoMode"));
    if (saved("aoHires", {"NSMBU_AO_HIRES"})) gfxvk::set_ao_hires(num("aoHires") != 0);
    if (saved("aniso", {"NSMBU_ANISO"})) gfxvk::set_aniso(num("aniso") != 0);
    if (saved("bloomStrength", {"NSMBU_BLOOM_STRENGTH"})) render::set_bloom_strength((float)num("bloomStrength"));
    if (saved("fxaa", {"NSMBU_FXAA"})) gfxvk::set_fxaa(num("fxaa") != 0);
    if (saved("interpFps", {"NSMBU_INTERP_FPS"})) interp::set_fps((int)num("interpFps"));
    if (saved("fps60", {"NSMBU_INTERP", "NSMBU_TRUE60", "NSMBU_INTERP_FPS"})) interp::set_mode((int)num("fps60"));
    if (saved("fps60Paced", {"NSMBU_INTERP_PACED"})) interp::set_paced_interpolation_at(60, num("fps60Paced") != 0);
    if (saved("fpsHighPaced", {"NSMBU_INTERP_PACED"})) interp::set_paced_interpolation_at(120, num("fpsHighPaced") != 0);
    if (saved("scaleFilter", {"NSMBU_SCALE_FILTER"})) gfxvk::set_scale_filter((int)num("scaleFilter"));
    if (saved("vkPresentMode", {"NSMBU_VK_PRESENT_MODE"})) {
        const int mode = (int)num("vkPresentMode");
        const bool chosen = v.count("vkPresentChosen") && v["vkPresentChosen"] == "1";
        if (chosen || mode != gfxvk::kPresentFifo) gfxvk::set_present_mode(mode);
    }

    using namespace gfx;
    if (v.count("drcMode")) {
        const int m = find_name(kModeNames, kDrcModeCount, v["drcMode"].c_str(), g_mode);
        if (drc_mode_offered(m)) g_mode = m;
    }
    if (v.count("pipCorner")) g_corner = find_name(kCornerNames, 4, v["pipCorner"].c_str(), g_corner);
    if (v.count("pipSize")) g_pip_size = std::clamp((float)num("pipSize"), 0.1f, 0.5f);
    if (v.count("pipOpacity")) g_pip_opacity = std::clamp((float)num("pipOpacity"), 0.2f, 1.0f);
    display_env_overrides();
    if (!drc_mode_offered(g_mode) && g_mode == kDrcWindow) g_mode = kDrcOff;
    g_shown = !input::pro_controller();
    apply_drc_window();
    LOG("[display] GamePad screen mode: %s%s", kModeNames[g_mode], g_shown ? "" : " (hidden)");
#ifndef __ANDROID__

    SDL_Window* tv = gfxvk::R.tv.window;
    if (tv && display_start_fullscreen(v["tvFullScreen"] == "1", SDL_GetWindowFlags(tv) & SDL_WINDOW_HIDDEN))
        set_fullscreen(true);
#endif
}

int scale_filter() { return gfxvk::scale_filter(); }
void set_scale_filter(int f) {
    gfxvk::set_scale_filter(f);
    graphics_changed();
}
bool scale_filter_available() { return gfxvk::graphics_feature_available(gfxvk::GraphicsFeature::ScaleFilter); }

bool fullscreen() { return gfxvk::R.tv.window && (SDL_GetWindowFlags(gfxvk::R.tv.window) & SDL_WINDOW_FULLSCREEN); }
void set_fullscreen(bool on) {
    if (!gfxvk::R.tv.window) return;
    if (!SDL_SetWindowFullscreen(gfxvk::R.tv.window, on)) LOG("[display] TV window: full screen %s failed: %s", on ? "on" : "off", SDL_GetError());
    tv_fullscreen_changed();
}

void tv_fullscreen_changed() {
#ifndef __ANDROID__
    if (gfx::display_fullscreen_env() || !gfxvk::R.tv.window) return;
    const std::string on = fullscreen() ? "1" : "0";
    {
        std::lock_guard<std::mutex> lk(g_mu);
        load_locked();
        auto it = g_values.find("tvFullScreen");
        if ((it == g_values.end() ? std::string("0") : it->second) == on) return;
        g_values["tvFullScreen"] = on;
        save_locked();
    }
    LOG("[display] TV window full screen %s: remembered for the next start", on == "1" ? "on" : "off");
#endif
}

int drc_modes() { return gfx::kDrcModeCount; }
bool drc_mode_offered(int m) { return gfx::drc_mode_offered(m); }
int drc_mode() { return gfx::g_mode; }
void set_drc_mode(int m) {
    if (!gfx::drc_mode_offered(m)) return;
    gfx::display_set_mode(m);
    apply_drc_window();
    save_display();
    LOG("[display] GamePad screen mode: %s", gfx::kModeNames[m]);
}
int pip_corner() { return gfx::g_corner; }
void set_pip_corner(int c) {
    gfx::g_corner = std::clamp(c, 0, 3);
    save_display();
}
float pip_size() { return gfx::g_pip_size; }
void set_pip_size(float s) {
    gfx::g_pip_size = std::clamp(s, 0.1f, 0.5f);
    save_display();
}
float pip_opacity() { return gfx::g_pip_opacity; }
void set_pip_opacity(float o) {
    gfx::g_pip_opacity = std::clamp(o, 0.2f, 1.0f);
    save_display();
}
bool drc_available() {
    return gfxvk::R.drc.window != nullptr || gfx::g_mode == gfx::kDrcPip || gfx::g_mode == gfx::kDrcAuto;
}
bool drc_shown() {
    SDL_Window* w = gfxvk::R.drc.window;
    return gfx::drc_screen_shown(w && !(SDL_GetWindowFlags(w) & SDL_WINDOW_HIDDEN));
}

void show_drc(bool on) {
    gfx::display_show_drc(on);
    apply_drc_window();
    LOG("[display] GamePad screen %s (%s)", on ? "shown" : "hidden", gfx::kModeNames[gfx::g_mode]);
}
void toggle_drc() { show_drc(!drc_shown()); }
void drc_window_closed() {
    if (gfx::g_mode == gfx::kDrcWindow) gfx::g_shown = false;
    apply_drc_window();
}
void set_pro_controller(bool on) {

    input::set_pro_controller(on);
    show_drc(!on);
}
const char* name() { return "SDL"; }
void set_clipboard(const std::string& text) { SDL_SetClipboardText(text.c_str()); }
#ifdef __ANDROID__
bool can_open_folder() { return false; }
void open_folder(const std::string&) {}
#else
bool can_open_folder() { return true; }
void open_folder(const std::string& path) {

    std::error_code ec;
    std::string p = std::filesystem::absolute(path, ec).generic_string();
    if (ec) p = path;
    std::string url = "file://";
    if (!p.empty() && p[0] != '/') url += "/";
    for (unsigned char c : p) {
        if (isalnum(c) || strchr("/-_.~:", c)) url += (char)c;
        else {
            char hex[4];
            snprintf(hex, sizeof hex, "%%%02X", c);
            url += hex;
        }
    }
    if (!SDL_OpenURL(url.c_str())) LOG("[overlay] cannot open %s: %s", url.c_str(), SDL_GetError());
}
#endif

}
#endif
