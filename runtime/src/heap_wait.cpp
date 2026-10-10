#include "runtime.h"

#include <atomic>
#include <chrono>
#include <thread>

extern "C" void f_02ACD218_orig(Cpu* c);

static thread_local int g_depth;

struct Depth {
    explicit Depth(int& d) : d(d) { ++d; }
    ~Depth() { --d; }
    int& d;
};

extern "C" void hook_02ACD218(Cpu* c) {
    if (g_depth > 0) {
        uint32_t ring = c->r[3] + 0x4A8u;
        if (ld32(ring) == 0) {
            static std::atomic<int> logged{0};
            if (logged.exchange(1) == 0)
                LOG("[heap] frame allocator waits for its buffer");
            BlockingScope block;
            while (ld32(ring) == 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    Depth depth(g_depth);
    f_02ACD218_orig(c);
}
