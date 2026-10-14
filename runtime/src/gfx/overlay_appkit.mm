

#import <AppKit/AppKit.h>

#include "../input.h"
#include "../overlay/hostui.h"
#include "../overlay/overlay.h"
#include "../runtime.h"
#include "display_modes.h"
#include <atomic>
#include <cstdlib>

namespace gfx {

float menu_res_scale();
void menu_set_res_scale(float f);
void menu_options_changed();

bool host_setting(const char* key, std::string& value);
void set_host_setting(const char* key, const std::string& value);
int display_filter();
void display_set_filter(int f);
int display_drc_mode();
void set_drc_mode(int m);
void display_set_pip(int corner, float size, float opacity);
bool tv_fullscreen();
void display_set_tv_fullscreen(bool on);
void* display_tv_window();
bool drc_window_available();
bool drc_window_shown();
void show_drc_window(bool on);
}

namespace hostui {
namespace {
std::atomic<bool> g_run_in_background{true};
std::atomic<bool> g_app_focused{true};
}

void post(std::function<void()> fn) {
    dispatch_async(dispatch_get_main_queue(), ^{ fn(); });
}
bool get(const char* key, std::string& value) { return gfx::host_setting(key, value); }
void set(const char* key, const std::string& value) { gfx::set_host_setting(key, value); }
float res_scale() { return gfx::menu_res_scale(); }
void set_res_scale(float s) { gfx::menu_set_res_scale(s); }
void graphics_changed() { gfx::menu_options_changed(); }
int scale_filter() { return gfx::display_filter(); }
void set_scale_filter(int f) { gfx::display_set_filter(f); }
bool scale_filter_available() { return true; }
bool fullscreen() { return gfx::tv_fullscreen(); }
void set_fullscreen(bool on) { gfx::display_set_tv_fullscreen(on); }
bool run_in_background() { return g_run_in_background.load(std::memory_order_relaxed); }
void set_run_in_background(bool on) {
    g_run_in_background.store(on, std::memory_order_relaxed);
    gfx::set_host_setting("runInBackground", on ? "1" : "0");
    LOG("[display] run in background %s", on ? "on" : "off");
}
bool app_focused() { return g_app_focused.load(std::memory_order_relaxed); }
void set_app_focused(bool on) { g_app_focused.store(on, std::memory_order_relaxed); }
int drc_modes() { return gfx::kDrcModeCount; }
bool drc_mode_offered(int m) { return m >= 0 && m < gfx::kDrcModeCount; }
int drc_mode() { return gfx::display_drc_mode(); }
void set_drc_mode(int m) { gfx::set_drc_mode(m); }
int pip_corner() { return gfx::g_corner; }
void set_pip_corner(int c) { gfx::display_set_pip(c, gfx::g_pip_size, gfx::g_pip_opacity); }
float pip_size() { return gfx::g_pip_size; }
void set_pip_size(float s) { gfx::display_set_pip(gfx::g_corner, s, gfx::g_pip_opacity); }
float pip_opacity() { return gfx::g_pip_opacity; }
void set_pip_opacity(float o) { gfx::display_set_pip(gfx::g_corner, gfx::g_pip_size, o); }
bool drc_available() { return gfx::drc_window_available(); }
bool drc_shown() { return gfx::drc_window_shown(); }
void show_drc(bool on) { gfx::show_drc_window(on); }
void set_pro_controller(bool on) {

    input::set_pro_controller(on);
    gfx::show_drc_window(!on);
}
const char* name() { return "AppKit"; }
void set_clipboard(const std::string& text) {
    NSPasteboard* pb = [NSPasteboard generalPasteboard];
    [pb clearContents];
    [pb setString:[NSString stringWithUTF8String:text.c_str()] forType:NSPasteboardTypeString];
}
bool can_open_folder() { return true; }
void open_folder(const std::string& path) {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:path.c_str()] isDirectory:YES]];
}
void run_posted() {}
void load_saved_options() {
    std::string v;
    if (const char* e = getenv("NSMBU_RUN_IN_BACKGROUND"))
        g_run_in_background.store(atoi(e) != 0, std::memory_order_relaxed);
    else if (gfx::host_setting("runInBackground", v))
        g_run_in_background.store(v != "0", std::memory_order_relaxed);
}
void toggle_drc() { show_drc(!drc_shown()); }
void drc_window_closed() {}
void tv_fullscreen_changed() {}
bool window_size(int&, int&) { return false; }
void window_resized() {}

}

namespace gfx {

void install_overlay_input() {
    NSWindow* tv = (__bridge NSWindow*)display_tv_window();
    if (!tv) return;
    auto density = [tv] { overlay::set_density((float)tv.backingScaleFactor); };
    density();
    [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidChangeBackingPropertiesNotification object:tv queue:nil
                                                  usingBlock:^(NSNotification*) { density(); }];
    if (getenv("NSMBU_NO_HOST_INPUT")) return;
    tv.acceptsMouseMovedEvents = YES;
    NSEventMask mask = NSEventMaskMouseMoved | NSEventMaskLeftMouseDragged | NSEventMaskRightMouseDragged | NSEventMaskOtherMouseDragged |
                       NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp | NSEventMaskRightMouseDown | NSEventMaskRightMouseUp |
                       NSEventMaskOtherMouseDown | NSEventMaskOtherMouseUp | NSEventMaskScrollWheel;
    [NSEvent addLocalMonitorForEventsMatchingMask:mask handler:^NSEvent*(NSEvent* e) {
        if (!overlay::captures() || e.window != tv) return e;
        NSView* v = tv.contentView;
        NSPoint p = [v convertPoint:e.locationInWindow fromView:nil];
        NSSize sz = v.bounds.size;
        if (sz.width <= 0 || sz.height <= 0) return e;

        if (p.y > sz.height && e.type != NSEventTypeScrollWheel) return e;
        overlay::mouse_move((float)(p.x / sz.width), (float)(v.isFlipped ? p.y / sz.height : 1.0 - p.y / sz.height));
        switch (e.type) {
        case NSEventTypeLeftMouseDown: overlay::mouse_button(0, true); break;
        case NSEventTypeLeftMouseUp: overlay::mouse_button(0, false); break;
        case NSEventTypeRightMouseDown: overlay::mouse_button(1, true); break;
        case NSEventTypeRightMouseUp: overlay::mouse_button(1, false); break;
        case NSEventTypeOtherMouseDown: overlay::mouse_button(2, true); break;
        case NSEventTypeOtherMouseUp: overlay::mouse_button(2, false); break;
        case NSEventTypeScrollWheel: {
            float dx = (float)e.scrollingDeltaX, dy = (float)e.scrollingDeltaY;
            if (e.hasPreciseScrollingDeltas) dx /= 10.0f, dy /= 10.0f;
            overlay::mouse_wheel(dx, dy);
            break;
        }
        default: break;
        }
        return nil;
    }];
}

}
