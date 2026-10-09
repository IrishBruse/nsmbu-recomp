#pragma once
#include <chrono>
#include <thread>
#if defined(_WIN32) && defined(NSMBU_SDL_HOST)
#include <SDL3/SDL_timer.h>
#endif

namespace host {
inline void sleep_until(std::chrono::steady_clock::time_point deadline) {
#if defined(_WIN32) && defined(NSMBU_SDL_HOST)

    const auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(
        deadline - std::chrono::steady_clock::now()).count();
    if (remaining > 0) SDL_DelayNS(static_cast<uint64_t>(remaining));
#else
    std::this_thread::sleep_until(deadline);
#endif
}
}
