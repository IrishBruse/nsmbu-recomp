#pragma once
#include <chrono>
#include <thread>
#if defined(_WIN32) && defined(NSMBU_SDL_HOST)
#include <SDL3/SDL_timer.h>
#elif defined(__linux__) && !defined(__ANDROID__)
#include <cerrno>
#include <ctime>
#include <sys/prctl.h>
#ifndef PR_SET_TIMERSLACK
#define PR_SET_TIMERSLACK 29
#endif
#endif

namespace host {
inline void sleep_until(std::chrono::steady_clock::time_point deadline) {
#if defined(_WIN32) && defined(NSMBU_SDL_HOST)

    const auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(
        deadline - std::chrono::steady_clock::now()).count();
    if (remaining > 0) SDL_DelayNS(static_cast<uint64_t>(remaining));
#elif defined(__linux__) && !defined(__ANDROID__)
    static thread_local bool slack = false;
    if (!slack) {
        prctl(PR_SET_TIMERSLACK, 1L);
        slack = true;
    }
    const auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(
        deadline - std::chrono::steady_clock::now()).count();
    if (remaining <= 0) return;
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += remaining / 1000000000;
    ts.tv_nsec += remaining % 1000000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }
    while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, nullptr) == EINTR) {}
#else
    std::this_thread::sleep_until(deadline);
#endif
}
}
