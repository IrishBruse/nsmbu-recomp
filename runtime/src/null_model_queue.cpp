#include "runtime.h"

#include <atomic>

extern "C" void f_024D752C_orig(Cpu* c);
extern "C" void f_029F1F4C(Cpu* c);

static void drop_null_models(Cpu* c, uint32_t list) {
    for (;;) {
        const uint32_t count = ld32(list);
        if (count == 0)
            return;
        const uint32_t arr = ld32(list + 8);
        if (arr == 0)
            return;
        bool removed = false;
        for (uint32_t i = 0; i < count; i++) {
            const uint32_t node = ld32(arr + i * 4);
            if (node == 0 || ld32(node + 0x28) != 0)
                continue;
            static std::atomic<int> logged{0};
            if (logged.exchange(1) == 0)
                LOG("[gfx] drop draw item with null model");
            const uint32_t free_head = ld32(list + 0xC);
            st32(node, free_head);
            st32(list + 0xC, node);
            const uint32_t save3 = c->r[3], save4 = c->r[4], save5 = c->r[5], save_lr = c->lr;
            c->r[3] = list;
            c->r[4] = i;
            c->r[5] = 1;
            f_029F1F4C(c);
            c->r[3] = save3;
            c->r[4] = save4;
            c->r[5] = save5;
            c->lr = save_lr;
            removed = true;
            break;
        }
        if (!removed)
            return;
    }
}

extern "C" void hook_024D752C(Cpu* c) {
    const uint32_t self = c->r[3];
    drop_null_models(c, self + 0x268u);
    drop_null_models(c, self + 0xE7Cu);
    drop_null_models(c, self + 0x1A90u);
    drop_null_models(c, self + 0x26A4u);
    f_024D752C_orig(c);
}
