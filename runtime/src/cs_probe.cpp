#include "runtime.h"

extern "C" void f_02994164_orig(Cpu* c);
extern "C" void nsmbu_note_scene_src(uint32_t src, uint32_t factory);
extern "C" void f_022656E0_orig(Cpu* c);

extern "C" void hook_02994164(Cpu* c) {
    static int logged = 0;
    if (logged++ < 200)
        LOG("[setnext] idx=%d scene=%08X src=%08X classid=%08X/%08X fader=%08X snd=%08X lr=%08X",
            logged - 1, c->r[3], c->r[4], ld32(c->r[5]), ld32(c->r[5] + 4), c->r[6], c->r[7], c->lr);
    nsmbu_note_scene_src(c->r[4], ld32(c->r[5] + 4));
    f_02994164_orig(c);
}

extern "C" void hook_029CBBB4(Cpu*) {}

extern "C" void hook_022656E0(Cpu* c) {
    static int logged = 0;
    if (logged++ < 5)
        LOG("[csstart] course select: lr=%08X this=%08X arg=%08X", c->lr, c->r[3], c->r[4]);
    f_022656E0_orig(c);
}
