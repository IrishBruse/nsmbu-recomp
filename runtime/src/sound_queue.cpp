#include "runtime.h"

#include <atomic>

extern "C" void f_024BD6EC_orig(Cpu* c);

extern "C" void hook_024BD6EC(Cpu* c) {
    if (c->r[5] == 0) {
        static std::atomic<int> logged{0};
        if (logged.exchange(1) == 0)
            LOG("[sound] drop request with no actor");
        c->r[3] = 0;
        return;
    }
    f_024BD6EC_orig(c);
}
