#import <AppKit/AppKit.h>
#include "../overlay/overlay.h"
#include "../motion/motion.h"
#include "mods.h"
#include "runtime.h"
#include <cstdlib>

namespace mods {
namespace {
NSWindow* __weak g_tv;
bool g_gyro_capture = false;

bool in_tv(NSEvent* e) { return g_tv && e.window == g_tv; }
}

bool mouse_captured() { return false; }

void update_gyro_mouse() {
    const bool want = g_tv && motion::mouse_drives_gyro() && !overlay::captures() && g_tv.isKeyWindow && NSApp.isActive &&
                      !getenv("NSMBU_NO_HOST_INPUT");
    if (want && !g_gyro_capture) {
        g_gyro_capture = true;
        CGAssociateMouseAndMouseCursorPosition(false);
        [NSCursor hide];
        LOG("[gyro] mouse captured while the game aims");
    } else if (!want && g_gyro_capture) {
        g_gyro_capture = false;
        CGAssociateMouseAndMouseCursorPosition(true);
        [NSCursor unhide];
    }
}

void mouse_release() {}

void mouse_init(void* tv_window) {
    g_tv = (__bridge NSWindow*)tv_window;
    if (getenv("NSMBU_NO_HOST_INPUT")) return;
    g_tv.acceptsMouseMovedEvents = YES;
    NSEventMask mask = NSEventMaskMouseMoved | NSEventMaskLeftMouseDragged | NSEventMaskRightMouseDragged |
                       NSEventMaskOtherMouseDragged;
    [NSEvent addLocalMonitorForEventsMatchingMask:mask handler:^NSEvent*(NSEvent* e) {
        if (overlay::captures()) return e;
        if ((e.type == NSEventTypeMouseMoved || e.type == NSEventTypeLeftMouseDragged || e.type == NSEventTypeRightMouseDragged ||
             e.type == NSEventTypeOtherMouseDragged) && (in_tv(e) || g_gyro_capture)) {
            motion::mouse_motion((float)e.deltaX, (float)e.deltaY);
            if (motion::mouse_drives_gyro()) return nil;
        }
        return e;
    }];
}

}
