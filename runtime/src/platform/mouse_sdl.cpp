#include "mouse_sdl.h"
#include "../motion/motion.h"
#include "../overlay/overlay.h"
#include "../runtime.h"
#include <cstdlib>

namespace mods {
namespace {
SDL_Window* tv = nullptr;
bool gyro_capture = false;
void update_gyro_capture() {
    const bool want = tv && motion::mouse_drives_gyro() && !overlay::captures() && SDL_GetKeyboardFocus() == tv &&
                      !getenv("NSMBU_NO_HOST_INPUT");
    if (want && !gyro_capture) {
        gyro_capture = true;
        if (SDL_SetWindowRelativeMouseMode(tv, true)) LOG("[gyro] mouse captured while the game aims");
    } else if (!want && gyro_capture) {
        gyro_capture = false;
        if (tv) SDL_SetWindowRelativeMouseMode(tv, false);
    }
}
}
bool mouse_captured() { return false; }
void mouse_release() {}
void mouse_init(void* window) {
    gyro_capture = false;
    tv = static_cast<SDL_Window*>(window);
}
void update_mouse() { update_gyro_capture(); }
bool handle_mouse_event(const SDL_Event& event) {
    update_mouse();
    if (!tv || getenv("NSMBU_NO_HOST_INPUT")) return false;
    const SDL_WindowID id = SDL_GetWindowID(tv);
    if (event.type == SDL_EVENT_MOUSE_MOTION && event.motion.windowID == id) {
        motion::mouse_motion(event.motion.xrel, event.motion.yrel);
        if (motion::mouse_drives_gyro()) return true;
    }
    return false;
}
}
