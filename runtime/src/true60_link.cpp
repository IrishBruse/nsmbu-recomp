

#include <cmath>

#include "runtime.h"
#include "true60.h"
#include "mods/mods.h"

extern "C" {
void f_02416230_orig(Cpu* c);
void f_0207A9A0_orig(Cpu* c);
void f_022ED850_orig(Cpu* c);
void f_02055B64_orig(Cpu* c);
void f_0211D2F8_orig(Cpu* c);
void f_025AAE08_orig(Cpu* c);
}

namespace {
constexpr uint32_t kSpeed = 0x33C;

bool link60() { return true60::dt() < 1.0f && true60::exec_proc() != 0 && true60::exec_proc() == true60::link(); }

uint32_t scratch_vec() {
    return mem::fixed_slot(mem::kFixLinkScratch);
}
}

extern "C" void site_023FCEB0(Cpu* c) {
    if (link60()) c->f[1].ps0 = (float)(c->f[1].ps0 / true60::dt());
}

extern "C" void site_023FD338(Cpu* c) {
    if (link60()) c->f[9].ps0 = (float)(c->f[9].ps0 * true60::dt());
}
extern "C" void site_023FD35C(Cpu* c) {
    if (link60()) c->f[9].ps0 = (float)(c->f[9].ps0 * true60::dt());
}

extern "C" void site_023FD39C(Cpu* c) {
    const bool half = link60();

    const float factor = 1.f;
    if (!half && factor == 1.f) return;
    const float dt = half ? true60::dt() : 1.f;
    uint32_t sp = c->r[4];
    float vx = u32_as_f32(ld32(sp)), vy = u32_as_f32(ld32(sp + 4)), vz = u32_as_f32(ld32(sp + 8));
    float dv = half ? vy - (float)c->f[31].ps0 : 0.f;
    uint32_t v = scratch_vec();
    st32(v, f32_as_u32(vx * dt * factor));
    st32(v + 4, half ? f32_as_u32(vy * dt + dv * (1.0f - dt) * 0.5f) : ld32(sp + 4));
    st32(v + 8, f32_as_u32(vz * dt * factor));
    c->r[4] = v;
}

extern "C" void hook_02416230(Cpu* c) {
    if (link60()) c->f[1].ps0 = (float)(c->f[1].ps0 * true60::dt());
    f_02416230_orig(c);
}

template <int kBytes>
static bool calc_timer_held(Cpu* c) {
    if (true60::dt() >= 1.0f || !true60::half_pass()) return false;
    uint32_t p = c->r[3];
    c->r[3] = kBytes == 1 ? ld8(p) : kBytes == 2 ? (uint32_t)(int32_t)(int16_t)ld16(p) : ld32(p);
    return true;
}
extern "C" void hook_0207A9A0(Cpu* c) { if (!calc_timer_held<1>(c)) f_0207A9A0_orig(c); }
extern "C" void hook_022ED850(Cpu* c) { if (!calc_timer_held<1>(c)) f_022ED850_orig(c); }
extern "C" void hook_02055B64(Cpu* c) { if (!calc_timer_held<2>(c)) f_02055B64_orig(c); }
extern "C" void hook_0211D2F8(Cpu* c) { if (!calc_timer_held<4>(c)) f_0211D2F8_orig(c); }
extern "C" void hook_025AAE08(Cpu* c) { if (!calc_timer_held<4>(c)) f_025AAE08_orig(c); }

namespace {
constexpr uint32_t kCurProc = 0x65F0;
constexpr uint32_t kM34C2 = 0x68DE;
constexpr uint32_t kResetFlg0 = 0x3C0;
constexpr uint32_t kReplayFlg0 = 0x2;
uint32_t g_noop = 0;
void noop_proc(Cpu* c) { c->r[3] = 1; }
struct ProcOut { uint64_t pass = ~0ull; uint32_t link = 0, flg0_pre = 0, flg0_set = 0; uint8_t m34c2 = 0; } g_out;
bool t_full_call = false;
void proc_call(Cpu* c) {
    uint32_t l = c->r[31];
    if (!true60::enabled() || l != true60::link()) return;
    if (link60()) {
        if (!g_noop) g_noop = dispatch::register_host(noop_proc, "true60_noop_proc");
        c->ctr = g_noop;
        if (g_out.link == l && g_out.pass + 1 == true60::pass()) {
            uint8_t m = g_out.m34c2;
            m = m == 3 ? 1 : (m == 4 || m == 7) ? 5 : m;
            if (ld8(l + kM34C2) == 0 && m) st8(l + kM34C2, m);
            st32(l + kResetFlg0, ld32(l + kResetFlg0) | (g_out.flg0_set & kReplayFlg0));
        }
        return;
    }
    if (true60::half_pass()) return;
    t_full_call = true;
    g_out.link = l;
    g_out.pass = ~0ull;
    g_out.flg0_pre = ld32(l + kResetFlg0);
}
}
extern "C" void site_0240D6D8(Cpu* c) { proc_call(c); }
extern "C" void site_0240D6F8(Cpu* c) { proc_call(c); }
extern "C" void site_0240D6FC(Cpu* c) {
    if (!t_full_call) return;
    t_full_call = false;
    uint32_t l = c->r[31];
    if (l != g_out.link) return;
    g_out.pass = true60::pass();
    g_out.flg0_set = ld32(l + kResetFlg0) & ~g_out.flg0_pre;
    g_out.m34c2 = ld8(l + kM34C2);
}

extern "C" void f_023FB230_orig(Cpu* c);
extern "C" void hook_023FB230(Cpu* c) {
    if (link60() && true60::half_pass()) return;
    f_023FB230_orig(c);
}

extern "C" void f_023FBCEC_orig(Cpu* c);
extern "C" void hook_023FBCEC(Cpu* c) {
    if (link60() && true60::half_pass()) return;
    f_023FBCEC_orig(c);
}

#define T60_FENCE_LINK(addr, ret)                                   \
    extern "C" void f_##addr##_orig(Cpu* c);                        \
    extern "C" void hook_##addr(Cpu* c) {                           \
        if (link60() && true60::half_pass()) { ret; return; }       \
        f_##addr##_orig(c);                                         \
    }
#define T60_FENCE_ANY(addr, ret)                                    \
    extern "C" void f_##addr##_orig(Cpu* c);                        \
    extern "C" void hook_##addr(Cpu* c) {                           \
        if (true60::enabled() && true60::preview()) { ret; return; }\
        f_##addr##_orig(c);                                         \
    }
T60_FENCE_LINK(023F695C, c->r[3] = 0)
T60_FENCE_LINK(023F7820, c->r[3] = 0)
T60_FENCE_LINK(023F81A4, c->r[3] = 0)
T60_FENCE_LINK(023F8F80, c->r[3] = 0)
T60_FENCE_LINK(023FA578, c->r[3] = 0)
T60_FENCE_LINK(023FB020, c->r[3] = 0)
T60_FENCE_LINK(023DBDD0, c->r[3] = 0)
T60_FENCE_LINK(023FD4E4, (void)0)
T60_FENCE_LINK(023DC7AC, (void)0)
T60_FENCE_ANY(025E14A8, c->r[3] = 0xFFFFFFFFu)
T60_FENCE_ANY(025DFAB8, c->r[3] = 0)
T60_FENCE_ANY(02821448, c->r[3] = 0)
T60_FENCE_ANY(0200E240, (void)0)
T60_FENCE_ANY(0253EC0C, c->r[3] = 0)
T60_FENCE_ANY(0253ED80, c->r[3] = 0)
T60_FENCE_ANY(025F0658, (void)0)
T60_FENCE_ANY(025B8AF4, (void)0)
T60_FENCE_ANY(025B51DC, (void)0)
