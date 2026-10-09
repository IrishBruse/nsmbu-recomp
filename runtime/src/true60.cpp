

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <filesystem>
#include <bit>
#include <string>
#include <vector>

#include "guest_addr.h"
#include "runtime.h"
#include "true60.h"

extern "C" {
void f_025DF940_orig(Cpu* c);
void f_025DF904_orig(Cpu* c);
void f_0200ECD4_orig(Cpu* c);
void f_0200ED84_orig(Cpu* c);
void f_0200EDC8_orig(Cpu* c);
void f_0200EE00_orig(Cpu* c);
void f_0200EF78_orig(Cpu* c);
void f_0200F164_orig(Cpu* c);
void f_0200F268_orig(Cpu* c);
void f_0200F378_orig(Cpu* c);
void f_0200F428_orig(Cpu* c);
void f_0200F474_orig(Cpu* c);
void f_0200F4FC_orig(Cpu* c);
void f_0200F564_orig(Cpu* c);
void f_0200F5C8_orig(Cpu* c);
void f_0200F62C_orig(Cpu* c);
void f_0200F764_orig(Cpu* c);
void f_0200F8D0_orig(Cpu* c);
void f_027F2FC4_orig(Cpu* c);
void f_025E3EC8_orig(Cpu* c);
void f_025D67A8_orig(Cpu* c);
void f_025D6800_orig(Cpu* c);
void f_025028B8_orig(Cpu* c);
void f_0282167C_orig(Cpu* c);
void f_025CB6D4_orig(Cpu* c);
void f_024EF968_orig(Cpu* c);
}

namespace true60_test {
void after_execute(uint32_t proc, uint32_t fn, bool is_link, float dt);
void before_execute_link(uint32_t proc);
uint64_t origin_step();
bool dumping();
}
namespace interp {
bool hold_pass();
uint64_t logic_steps();
}

namespace true60 {

static std::atomic<bool> g_on{[] { const char* e = getenv("NSMBU_TRUE60"); return e && atoi(e) != 0; }()};
bool enabled() { return g_on.load(std::memory_order_relaxed); }
void set_enabled(bool v) {
    g_on = v;
    LOG("[true60] game logic at 60 steps/s %s", v ? "on" : "off");
}

thread_local float t_dt = 1.0f;
thread_local uint32_t t_proc = 0;
float dt() { return t_dt; }
uint32_t exec_proc() { return t_proc; }
bool half_pass() { return interp::hold_pass(); }

uint64_t g_pass = 0;
int g_link_ride = 0;
int g_cam_fallback = 0;
void new_pass() {
    g_pass++;
    if (g_cam_fallback > 0) g_cam_fallback--;
    if (g_link_ride > 0) g_link_ride--;
}
uint64_t pass() { return g_pass; }

int32_t split(int32_t v) {
    if (t_dt >= 1.0f) return v;
    int32_t a = v >= 0 ? (v + 1) / 2 : -((-v + 1) / 2);
    return half_pass() ? a : v - a;
}

static double frac(double s) {
    if (t_dt >= 1.0f || s <= 0.0 || s >= 1.0) return s;
    return 1.0 - std::pow(1.0 - s, (double)t_dt);
}

thread_local int t_saved_reg = -1;
thread_local double t_saved = 0;
void ratio_begin(Cpu* c, int r) {
    if (t_dt >= 1.0f) return;
    t_saved_reg = r;
    t_saved = c->f[r].ps0;
    c->f[r].ps0 = (float)frac(c->f[r].ps0);
}
void ratio_end(Cpu* c, int r) {
    if (t_saved_reg != r) return;
    c->f[r].ps0 = t_saved;
    t_saved_reg = -1;
}
void ratio_arg(Cpu* c, int r) {
    if (t_dt < 1.0f) c->f[r].ps0 = (float)frac(c->f[r].ps0);
}

namespace {
const uint32_t kRndSeeds = GD(0x101FF9D4);
constexpr uint32_t kSubMethod = 0xF0;
const uint32_t kDaPyExecute = GC(0x0240EBB0);
constexpr uint32_t kCurProc = 0x65F0;
constexpr uint32_t kPos = 0x314, kOld = 0x300, kSpeed = 0x33C, kSpeedF = 0x370, kGravity = 0x374, kMaxFall = 0x378;
constexpr uint32_t kShapeAngle = 0x328, kAngle = 0x320;
constexpr uint32_t kFrameCtrlUnder = 0x5898;
constexpr uint32_t kNormalSpeed = 0x6A14;

struct ProcInfo {
    uint64_t pass = 0;
    float dt = 1.0f;
};
std::unordered_map<uint32_t, ProcInfo> g_procs;
uint32_t g_link = 0;
uint64_t g_link_steps = 0;

uint32_t actor_execute_fn(uint32_t proc) {
    uint32_t sub = ld32(proc + kSubMethod);
    if (sub < 0x10000000 || sub >= 0x50000000) return 0;
    return ld32(sub + 8);
}

const char* const kGroupNames[kNumGroups] = {"loco", "camera", "sword", "items", "swim", "sail", "bk", "mo2", "cc", "ki"};
constexpr uint32_t kGroupDefault = (1u << kGrpLoco) | (1u << kGrpCamera) | (1u << kGrpSword);
uint32_t g_groups = [] {
    uint32_t m = kGroupDefault;
    const char* e = getenv("NSMBU_TRUE60_GROUPS");
    bool replaced = false;
    for (const char* p = e; p && *p;) {
        size_t n = strcspn(p, ",");
        std::string w(p, n);
        p += n + (p[n] == ',');
        char sign = w.empty() ? 0 : w[0];
        if (sign == '+' || sign == '-') w = w.substr(1);
        else if (!replaced) m = 0, replaced = true;
        uint32_t bit = 0;
        if (w == "all") bit = (1u << kNumGroups) - 1;
        else if (w == "none") bit = 0, m = sign == '+' ? m : 0;
        for (int g = 0; g < kNumGroups; g++)
            if (w == kGroupNames[g]) bit = 1u << g;
        if (sign == '-') m &= ~bit;
        else m |= bit;
    }
    std::string on;
    for (int g = 0; g < kNumGroups; g++)
        if (m & (1u << g)) on += std::string(on.empty() ? "" : ",") + kGroupNames[g];
    LOG("[true60] groups: %s", on.c_str());
    return m;
}();

uint8_t g_link_proc[256];
const int kAudited[] = {

    0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x12, 0x17,
    0x18, 0x19, 0x1C, 0x1D, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
    0x2E, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x38, 0x39, 0x3A, 0x3B, 0x3C,

    0x3D, 0x3E, 0x3F, 0x41, 0x42, 0x43,
    0x44, 0x47, 0x48, 0x4A, 0x4B, 0x4C,
    0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52,
    0x53, 0x54, 0x55, 0x57, 0x58, 0x5A,
    0x5B, 0x5C, 0x5E, 0x5F, 0x62, 0x63,
    0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x6B, 0x6C, 0x6D, 0x80, 0x81,
    0x82, 0x94, 0x95, 0x96, 0x98, 0x99,
    0x9C, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2,
    0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8,
};
struct ProcGroup { Group g; uint8_t mode; std::vector<int> procs; };
const ProcGroup kLinkGroups[] = {

    {kGrpLoco, 1, {
        0x06,
        0x07,
        0x0A,
        0x22,
        0x24,
        0x27,
    }},

    {kGrpLoco, 2, {
        0x04,
        0x05,
        0x1E,
        0x23,
        0x25,
    }},

    {kGrpSword, 2, {
        0x41, 0x42, 0x43, 0x44,
        0x45, 0x46,
        0x47, 0x48,
        0x4A,
        0x55, 0x58, 0x59,
        0x5A,
    }},

    {kGrpItems, 2, {
        0x80, 0x81, 0x82,
        0x83, 0x84,
        0x94, 0x95,
    }},
};
void init_link_procs() {
    static bool done = false;
    if (done) return;
    done = true;
    const char* e = getenv("NSMBU_TRUE60_LINK");
    if (e && !strcmp(e, "audited")) {
        for (int v : kAudited) g_link_proc[v] = 1;
        for (auto& pg : kLinkGroups)
            for (int v : pg.procs) g_link_proc[v] = pg.mode;
    } else if (e && !strcmp(e, "all")) {
        for (auto& b : g_link_proc) b = 1;
    } else if (e && strcmp(e, "none")) {
        for (const char* p = e; *p;) {
            char* end;
            long v = strtol(p, &end, 0);
            if (end == p) break;
            uint8_t mode = 1;
            if (*end == 's') mode = 2, end++;
            if (v >= 0 && v < 256) g_link_proc[v] = mode;
            p = *end == ',' ? end + 1 : end;
        }
    } else if (!e) {
        for (auto& pg : kLinkGroups)
            if (g_groups & (1u << pg.g))
                for (int v : pg.procs) g_link_proc[v] = pg.mode;
    }
}

constexpr uint32_t kCamMtd = 0x228;
const uint32_t kCameraExecute = GC(0x024FFA3C);
constexpr uint32_t kDCamera = 0x248;
bool g_follow_called = false;
bool g_cam_follow = false;
uint32_t g_camera = 0;
bool camera_enabled() {
    static const bool on = (!getenv("NSMBU_TRUE60_CAMERA") || atoi(getenv("NSMBU_TRUE60_CAMERA")) != 0) && (g_groups & (1u << kGrpCamera));
    return on && g_cam_fallback == 0;
}

constexpr uint32_t kCamSnap = 0xB28;
bool g_cam_step = false;
bool g_cam_nan = false;
uint32_t g_cam_nan_lr = 0;
bool cam_has_nan(uint32_t proc) {
    for (uint32_t o = kDCamera; o < kCamSnap; o += 4) {
        uint32_t v = ld32(proc + o);
        if ((v & 0x7F800000) == 0x7F800000 && (v & 0x007FFFFF)) {

            return true;
        }
    }
    return false;
}
bool is_camera(uint32_t proc) {
    uint32_t m = ld32(proc + kCamMtd);
    return m >= 0x10000000 && m < 0x50000000 && ld32(m + 8) == kCameraExecute;
}

float classify(uint32_t proc) {
    static bool init = (init_link_procs(), true);
    (void)init;
    uint32_t fn = actor_execute_fn(proc);
    if (fn == kDaPyExecute) {
        if (g_link != proc) {
            g_link = proc;
            LOG("[true60] Link is process %08X", proc);
            if (false) {
                uint32_t of = ld32(proc + 0x65CC);
                uint32_t ti = ld32(of + 0x1C), q = ld32(of + 0x20);
                LOG("[t60dbg] old_fdata %08X trans %08X quat %08X (q-t %X); headers %08X %08X %08X %08X / %08X %08X %08X %08X", of, ti, q, q - ti,
                    ld32(ti - 16), ld32(ti - 12), ld32(ti - 8), ld32(ti - 4), ld32(q - 16), ld32(q - 12), ld32(q - 8), ld32(q - 4));
                LOG("[t60dbg] Link header %08X %08X %08X %08X", ld32(proc - 16), ld32(proc - 12), ld32(proc - 8), ld32(proc - 4));
            }
        }
        uint32_t p = ld32(proc + kCurProc);

        if (g_link_ride > 0) return 1.0f;
        return p < 256 && g_link_proc[p] ? 0.5f : 1.0f;
    }
    if (is_camera(proc)) {
        if (g_camera != proc) LOG("[true60] camera is process %08X", proc);
        g_camera = proc;
        return camera_enabled() && g_cam_follow ? 0.5f : 1.0f;
    }
    return 1.0f;
}

FILE* g_trace = [] {
    const char* p = getenv("NSMBU_LINK_TRACE");
    return p ? fopen(p, "w") : nullptr;
}();
void trace_link(uint32_t proc, float dt) {
    if (!g_trace) return;
    double t = (double)timebase::now() / timebase::kTicksPerSec;
    fprintf(g_trace, "%llu %llu %.4f %d %.2f %d %.3f %.3f %.3f %.4f %.4f %.4f %.4f %d %d %.3f %.3f %.3f\n", (unsigned long long)g_pass, (unsigned long long)interp::logic_steps(), t,
            half_pass() ? 0 : 1, dt, (int)ld32(proc + kCurProc), ldf32(proc + kPos), ldf32(proc + kPos + 4), ldf32(proc + kPos + 8),
            ldf32(proc + kSpeedF), ldf32(proc + kSpeed), ldf32(proc + kSpeed + 4), ldf32(proc + kSpeed + 8),
            (int)(int16_t)ld16(proc + kShapeAngle + 2), (int)(int16_t)ld16(proc + kAngle + 2),
            ldf32(proc + kFrameCtrlUnder + 4), ldf32(proc + kFrameCtrlUnder), ldf32(proc + kNormalSpeed));
    static int n = 0;
    if (++n % 60 == 0) fflush(g_trace);
}

FILE* g_cam_trace = [] {
    const char* p = getenv("NSMBU_CAM_TRACE");
    return p ? fopen(p, "w") : nullptr;
}();
void trace_camera(uint32_t proc, float dt) {
    if (!g_cam_trace) return;
    fprintf(g_cam_trace, "%llu %llu %d %.2f %d %.3f %.3f %.3f %.3f %.3f %.3f %.3f\n", (unsigned long long)g_pass,
            (unsigned long long)interp::logic_steps(), half_pass() ? 0 : 1, dt, g_follow_called ? 1 : 0, ldf32(proc + 0xDC),
            ldf32(proc + 0xE0), ldf32(proc + 0xE4), ldf32(proc + 0xE8), ldf32(proc + 0xEC), ldf32(proc + 0xF0), ldf32(proc + 0xD4));

    uint32_t d = proc + kDCamera;
    fprintf(g_cam_trace, "    m100 %d m108 %d work", (int)ld32(d + 0x100), (int)ld32(d + 0x108));
    for (uint32_t o = 0x36C; o <= 0x388; o += 4) fprintf(g_cam_trace, " %08X", ld32(d + o));
    fprintf(g_cam_trace, "\n");
    static int n = 0;
    if (++n % 60 == 0) fflush(g_cam_trace);
}
}

constexpr uint32_t kLinkSize = 0x8284;
constexpr uint32_t kActorHeap = 0xF4;

struct Region { uint32_t base = 0; std::vector<uint32_t> before, after; };
struct LinkSnap {
    uint64_t pass = ~0ull;
    uint32_t proc = 0;
    std::vector<uint8_t> link;
    Region regions[4];
    uint32_t env_player[3];
} g_snap;
const uint32_t kEnvPlayerPos = GD(0x10475A68) + 0xB2C;
void region_begin(Region& r, uint32_t base, uint32_t size) {
    r.base = base;
    r.before.resize(size / 4);
    memcpy(r.before.data(), ppc_ptr(base), size & ~3u);
}
void region_end(Region& r) {
    if (!r.base) return;
    r.after.resize(r.before.size());
    memcpy(r.after.data(), ppc_ptr(r.base), 4 * r.after.size());
}
void region_restore(Region& r) {
    if (!r.base || r.after.size() != r.before.size()) return;
    static const bool dbg = getenv("NSMBU_T60_REGIONLOG") != nullptr;
    if (dbg && (&r == &g_snap.regions[3])) {
        static std::unordered_map<uint32_t, int> cnt;
        static int n = 0;
        for (size_t i = 0; i < r.before.size(); i++)
            if (r.after[i] != r.before[i]) cnt[r.base + 4 * (uint32_t)i]++;
        if (++n % 100 == 0) {
            std::vector<std::pair<uint32_t, int>> v(cnt.begin(), cnt.end());
            std::sort(v.begin(), v.end());
            std::string o;
            for (auto& [a, c] : v) { char t[32]; snprintf(t, sizeof t, " %08X:%d", a, c); o += t; }
            LOG("[regionlog]%s", o.c_str());
        }
    }
    if (dbg && &r == &g_snap.regions[3]) return;
    uint32_t* cur = (uint32_t*)ppc_ptr(r.base);
    const uint32_t* b = r.before.data();
    const uint32_t* a = r.after.data();
    for (size_t i = 0, n = r.before.size(); i < n; i++)
        if (a[i] != b[i]) cur[i] = b[i];
}

std::vector<uint8_t> g_memdiff;
uint64_t g_memdiff_step = getenv("NSMBU_T60_MEMDIFF") ? strtoull(getenv("NSMBU_T60_MEMDIFF"), nullptr, 10) : 0;
const uint32_t kMemDiffLo = getenv("NSMBU_T60_MEMDIFF_LO") ? (uint32_t)strtoul(getenv("NSMBU_T60_MEMDIFF_LO"), nullptr, 16) : 0x10000000;
const uint32_t kMemDiffHi = getenv("NSMBU_T60_MEMDIFF_HI") ? (uint32_t)strtoul(getenv("NSMBU_T60_MEMDIFF_HI"), nullptr, 16) : 0x4A000000;
void link_preview_begin(uint32_t proc) {
    if (g_memdiff_step && true60_test::origin_step() && interp::logic_steps() == true60_test::origin_step() + g_memdiff_step && g_memdiff.empty()) {
        g_memdiff.assign(ppc_ptr(kMemDiffLo), ppc_ptr(kMemDiffHi));
        LOG("[memdiff] snapshot at step %llu", (unsigned long long)g_memdiff_step);
    }
    g_snap.pass = g_pass;
    g_snap.proc = proc;
    g_snap.link.assign(ppc_ptr(proc), ppc_ptr(proc) + kLinkSize);
    for (int i = 0; i < 3; i++) g_snap.env_player[i] = ld32(kEnvPlayerPos + 4 * i);
    for (auto& r : g_snap.regions) r.base = 0;
    region_begin(g_snap.regions[0], GD(0x1046CD10), 0x44);
    region_begin(g_snap.regions[1], GD(0x10473FE8), GD(0x10474C68) - GD(0x10473FE8));
    if (const char* e = getenv("NSMBU_T60_EXTRA_REGION")) {
        uint32_t lo = (uint32_t)strtoul(e, nullptr, 16), sz = strchr(e, ':') ? (uint32_t)strtoul(strchr(e, ':') + 1, nullptr, 16) : 0;
        if (sz) region_begin(g_snap.regions[3], lo, sz);
    }
    uint32_t h = ld32(proc + kActorHeap);
    if (h >= mem::kMem2Start && h < mem::kMem2End) {
        uint32_t start = ld32(h + 0x8), size = ld32(h + 0x20) & ~3u;
        if (start >= mem::kMem2Start && (start & 3) == 0 && size > 0 && size <= 0x200000 && start + size <= mem::kMem2End)
            region_begin(g_snap.regions[2], h, start + size - h);
    }
    static bool warned = false;
    if (!g_snap.regions[2].base && !warned) {
        warned = true;
        LOG("[true60] Link's actor heap not found (%08X)", h);
    }
}
void link_preview_end() {
    for (auto& r : g_snap.regions) region_end(r);
}
void link_preview_restore() {
    if (g_snap.pass == ~0ull) return;
    g_snap.pass = ~0ull;
    if (g_snap.proc != g_link || g_snap.link.size() != kLinkSize) return;
    memcpy(ppc_ptr(g_snap.proc), g_snap.link.data(), kLinkSize);
    for (auto& r : g_snap.regions) region_restore(r);

    for (int i = 0; i < 3; i++) st32(kEnvPlayerPos + 4 * i, g_snap.env_player[i]);
}

const bool g_cam_exact = !getenv("NSMBU_TRUE60_CAMERA_EXACT") || atoi(getenv("NSMBU_TRUE60_CAMERA_EXACT")) != 0;
std::vector<uint8_t> g_cam_half;

const uint32_t kPlayCamLo = GD(0x1046F0B0), kPlayCamHi = GD(0x10485000);
Region g_cam_play;
uint64_t g_cam_half_pass = ~0ull, g_cam_restored = ~0ull;
uint32_t g_cam_half_proc = 0;
struct TimerField { uint32_t off; int bytes; };

const TimerField kCamCounters[] = {
    {kDCamera + 0x07C, 4}, {kDCamera + 0x080, 4}, {kDCamera + 0x108, 4}, {kDCamera + 0x118, 4}, {kDCamera + 0x11C, 4},
    {kDCamera + 0x138, 4}, {kDCamera + 0x380, 4}, {kDCamera + 0x38A, 2},
};
constexpr int kNumCamCounters = sizeof(kCamCounters) / sizeof(kCamCounters[0]);
constexpr int kNumCamStepCounters = 5;
int32_t read_field(uint32_t a, int bytes) { return bytes == 2 ? (int32_t)(int16_t)ld16(a) : (int32_t)ld32(a); }
void write_field(uint32_t a, int bytes, int32_t v) {
    if (bytes == 2) st16(a, (uint16_t)v);
    else st32(a, (uint32_t)v);
}

bool group_on(Group g) { return (g_groups >> g) & 1; }
int link_proc_mode(uint32_t p) {
    init_link_procs();
    return p < 256 ? g_link_proc[p] : 0;
}
float set_dt(float dt) {
    float o = t_dt;
    t_dt = dt;
    return o;
}

bool runs_60(uint32_t proc) {
    if (!enabled()) return false;
    auto it = g_procs.find(proc);
    return it != g_procs.end() && it->second.pass == g_pass && it->second.dt < 1.0f;
}
uint32_t link() { return g_link; }
uint64_t link_steps() { return g_link_steps; }
bool g_state_loaded = false;
bool state_loaded() { return g_state_loaded; }

Region g_camdraw;
void camera_draw_preview(bool) {}

struct HalfSave { uint32_t addr, size; std::vector<uint8_t> data; };

HalfSave g_half_saves[] = {
    {GD(0x10474DE0), 0x10, {}},
    {GD(0x1047E720), 0x90, {}},
    {GD(0x104846B0), 0x90, {}},
    {GD(0x1048CFF0), 0x80, {}},
    {GD(0x104B45F8), 0x30, {}},
};
uint64_t g_half_saves_pass = ~0ull;
void pass_begin(bool full) {
    static const bool keep = getenv("NSMBU_TRUE60_DRAWSTATE_KEEP") != nullptr;
    if (!full) {
        if (enabled() && !keep && (g_link || g_camera)) {
            for (auto& h : g_half_saves) h.data.assign(ppc_ptr(h.addr), ppc_ptr(h.addr) + h.size);
            g_half_saves_pass = g_pass;
        }
        return;
    }
    if (g_half_saves_pass + 1 == g_pass)
        for (auto& h : g_half_saves) memcpy(ppc_ptr(h.addr), h.data.data(), h.size);
    g_half_saves_pass = ~0ull;
    link_preview_restore();

    if (!g_memdiff.empty() && g_memdiff_step) {
        g_memdiff_step = 0;
        int n = 0;
        size_t i = 0, sz = g_memdiff.size();
        const uint8_t* cur = ppc_ptr(kMemDiffLo);
        while (i < sz && n < 400) {
            if (cur[i] == g_memdiff[i]) { i++; continue; }
            size_t j = i;
            size_t last = i;
            while (j < sz && j - last < 32) { if (cur[j] != g_memdiff[j]) last = j; j++; }
            LOG("[memdiff] %08X +%zX", (uint32_t)(kMemDiffLo + i), last + 1 - i);
            n++;
            i = last + 1;
        }
        g_memdiff.clear();
        g_memdiff.shrink_to_fit();
    }

    if (g_cam_half_pass + 1 == g_pass && g_cam_half_proc == g_camera && g_cam_half.size() == kCamSnap) {
        memcpy(ppc_ptr(g_cam_half_proc), g_cam_half.data(), kCamSnap);
        region_restore(g_cam_play);
        g_cam_restored = g_pass;
    }
    g_cam_play.base = 0;
    g_cam_half_pass = ~0ull;
}
bool preview() { return t_dt < 1.0f && half_pass(); }
void ss_reset() {
    g_link_steps = 0;
    g_state_loaded = true;
    g_snap.pass = ~0ull;
    g_cam_half_pass = ~0ull;
    g_procs.clear();
    g_link = 0;
    g_camera = 0;
    g_link_ride = 0;
    g_follow_called = g_cam_follow = false;
}

namespace {
struct Stats { uint32_t full = 0, half = 0, skipped = 0, link60 = 0, link30 = 0, particles = 0; } g_st;
void stats_tick() {
    static uint64_t t0 = timebase::now();
    uint64_t t = timebase::now();
    if (t - t0 < 10 * timebase::kTicksPerSec) return;
    double s = (double)(t - t0) / timebase::kTicksPerSec;
    LOG("[true60] per second: %.1f executes at dt=1, %.1f at dt=0.5, %.1f held back; Link %.1f steps at 60 Hz + %.1f at 30 Hz; "
        "particle updates %.1f",
        g_st.full / s, g_st.half / s, g_st.skipped / s, g_st.link60 / s, g_st.link30 / s, g_st.particles / s);
    g_st = Stats{};
    t0 = t;
}
}

}

using namespace true60;

static uint32_t watch_addr() {
    static const long abs_a = getenv("NSMBU_WATCH_ADDR") ? strtol(getenv("NSMBU_WATCH_ADDR"), nullptr, 16) : -1;
    if (abs_a >= 0) return (uint32_t)abs_a;
    static const long off = getenv("NSMBU_WATCH_LINK") ? strtol(getenv("NSMBU_WATCH_LINK"), nullptr, 16) : -1;
    return off >= 0 && true60::link() ? true60::link() + (uint32_t)off : 0;
}
static void watch_link(const char* what, uint32_t proc, uint32_t before) {
    uint32_t wa = watch_addr();
    if (!wa) return;
    uint32_t v = ld32(wa);
    if (v != before) LOG("[watch] pass %llu step %llu %s %s %08X (exec fn %08X): %08X -> %08X", (unsigned long long)true60::pass(), (unsigned long long)interp::logic_steps(), interp::hold_pass() ? "half" : "full", what, proc, actor_execute_fn(proc), before, v);
}
static uint32_t watch_val() {
    uint32_t wa = watch_addr();
    return wa ? ld32(wa) : 0;
}
static FILE* g_rx = getenv("NSMBU_RND_EXEC") ? fopen(getenv("NSMBU_RND_EXEC"), "w") : nullptr;
static void rx_log(uint32_t proc, uint32_t s0) {
    if (g_rx && ld32(kRndSeeds) != s0)
        fprintf(g_rx, "%llu %d %08X %08X %08X\n", (unsigned long long)interp::logic_steps(), (int)half_pass(), proc, actor_execute_fn(proc), s0);
}
extern "C" void hook_025DF940(Cpu* c) {
    uint32_t proc = c->r[3];
    uint32_t rx_s0 = ld32(kRndSeeds);
    struct WatchGuard { uint32_t p, v; ~WatchGuard() { watch_link("execute", p, v); } } watch_guard{proc, watch_val()};
    struct RxLog { uint32_t p, s; ~RxLog() { rx_log(p, s); } } rx_guard{proc, rx_s0};
    if (!enabled()) {
        bool is_link = g_trace && actor_execute_fn(proc) == kDaPyExecute;
        bool is_cam = g_cam_trace && is_camera(proc);
        if (is_cam) g_follow_called = false;
        uint32_t fn = true60_test::dumping() ? actor_execute_fn(proc) : 0;
        if (fn == kDaPyExecute) true60_test::before_execute_link(proc);
        f_025DF940_orig(c);
        if (fn) true60_test::after_execute(proc, fn, fn == kDaPyExecute, 1.0f);
        if (is_cam) trace_camera(proc, 1.0f);
        g_st.full++;
        if (g_trace || g_cam_trace) stats_tick();
        if (is_link) {
            g_link = proc;
            trace_link(proc, 1.0f);
        }
        if (actor_execute_fn(proc) == kDaPyExecute) g_link_steps++;
        return;
    }
    float dt = classify(proc);
    if (half_pass() && dt >= 1.0f) {
        g_st.skipped++;
        c->r[3] = 0;
        return;
    }

    bool link_conv = proc == g_link && dt < 1.0f;
    if (link_conv && !half_pass()) dt = 1.0f;
    bool link_preview = link_conv && half_pass();
    if (link_preview) link_preview_begin(proc);

    bool cam_preview = false;
    if (g_cam_exact && proc == g_camera && dt < 1.0f) {
        if (half_pass()) {
            g_cam_half.assign(ppc_ptr(proc), ppc_ptr(proc) + kCamSnap);
            g_cam_half_pass = g_pass;
            g_cam_half_proc = proc;
            cam_preview = true;
            region_begin(g_cam_play, kPlayCamLo, kPlayCamHi - kPlayCamLo);
        } else if (g_cam_restored == g_pass) {
            dt = 1.0f;
            link_conv = true;
        }
    }
    float saved_dt = t_dt;
    uint32_t saved_proc = t_proc;
    t_dt = dt;
    t_proc = proc;
    int32_t cam[kNumCamCounters];
    bool hold_cam = dt < 1.0f && half_pass() && proc == g_camera;
    uint32_t cam_sum = 0;
    if (hold_cam) {
        for (int i = 0; i < kNumCamCounters; i++) cam[i] = read_field(proc + kCamCounters[i].off, kCamCounters[i].bytes);
        cam_sum = ld32(proc + kDCamera + 0x384);
    }
    if (proc == g_camera) g_follow_called = false;
    static std::vector<uint8_t> snap;
    bool cam60 = proc == g_camera;
    bool nan_before = false;
    if (cam60) {
        snap.resize(kCamSnap);
        memcpy(snap.data(), ppc_ptr(proc), kCamSnap);
        nan_before = cam_has_nan(proc);
        g_cam_step = true;
        g_cam_nan = false;
    }
    uint32_t dump_fn = true60_test::dumping() ? actor_execute_fn(proc) : 0;
    if (dump_fn == kDaPyExecute) true60_test::before_execute_link(proc);

    uint32_t rng[3];
    bool hold_rng = dt < 1.0f && half_pass();
    if (hold_rng)
        for (int i = 0; i < 3; i++) rng[i] = ld32(kRndSeeds + 4 * i);
    f_025DF940_orig(c);
    if (link_preview) link_preview_end();
    if (cam_preview) region_end(g_cam_play);
    if (hold_rng)
        for (int i = 0; i < 3; i++) st32(kRndSeeds + 4 * i, rng[i]);
    if (cam60) {
        g_cam_step = false;
        if (g_cam_nan || (!nan_before && cam_has_nan(proc))) {
            static int logged = 0;
            if (logged++ < 20) {
                std::error_code directory_error;
                std::filesystem::create_directory("captures", directory_error);
                if (FILE* f = fopen("captures/true60-nan.log", "a")) {
                    fprintf(f, "pass %llu: 60 Hz camera step produced NaN (sphere %d, caller %08X); restored, 30 Hz for 60 passes\n",
                            (unsigned long long)g_pass, (int)g_cam_nan, g_cam_nan_lr);
                    for (uint32_t o = kDCamera; o < kCamSnap; o += 4) {
                        uint32_t v = ld32(proc + o), b;
                        memcpy(&b, snap.data() + o, 4);
                        b = __builtin_bswap32(b);
                        if ((v & 0x7F800000) == 0x7F800000 && (v & 0x007FFFFF))
                            fprintf(f, "  dCamera+0x%03X: %08X (before %08X = %g)\n", o - kDCamera, v, b, (double)std::bit_cast<float>(b));
                    }

                    if (uint32_t l = g_link) {
                        fprintf(f, "  dt %.2f, half pass %d, Link %08X proc %u at 60 Hz %d; Link NaN fields:", dt, (int)half_pass(), l,
                                ld32(l + kCurProc), (int)runs_60(l));
                        int n = 0;
                        for (uint32_t o = 0; o < 0x7000 && n < 40; o += 4) {
                            uint32_t v = ld32(l + o);
                            if (v == 0x7FC00000 || v == 0xFFC00000) fprintf(f, " +0x%X", o), n++;
                        }
                        fprintf(f, "\n");
                    }
                    fclose(f);
                }
                LOG("[true60] camera step produced NaN: restored, camera at 30 Hz for a second (captures/true60-nan.log)");
            }
            memcpy(ppc_ptr(proc), snap.data(), kCamSnap);
            g_cam_fallback = 60;
        }
    }
    if (proc == g_camera) {
        g_cam_follow = g_follow_called;
        trace_camera(proc, dt);
    }
    if (hold_cam) {

        bool restarted = read_field(proc + kDCamera + 0x108, 4) < cam[2];
        if (!restarted && ld32(proc + kDCamera + 0x37C) == 0x464C4C57 && ld32(proc + kDCamera + 0x384) != cam_sum)
            st32(proc + kDCamera + 0x384, cam_sum);
        for (int i = 0; i < kNumCamCounters; i++) {
            uint32_t a = proc + kCamCounters[i].off;
            int32_t v = read_field(a, kCamCounters[i].bytes);

            bool up_only = i < kNumCamStepCounters;
            if (v == cam[i] + 1 || (!up_only && v == cam[i] - 1)) write_field(a, kCamCounters[i].bytes, cam[i]);
        }
    }
    t_dt = saved_dt;
    t_proc = saved_proc;
    if (dump_fn) true60_test::after_execute(proc, dump_fn, dump_fn == kDaPyExecute, dt);
    if (proc == g_link && !half_pass()) g_link_steps++;
    ProcInfo& pi = g_procs[proc];
    pi.pass = g_pass;
    pi.dt = link_conv ? 0.5f : dt;
    (dt < 1.0f ? g_st.half : g_st.full)++;
    if (proc == g_link) {
        (dt < 1.0f ? g_st.link60 : g_st.link30)++;
        trace_link(proc, dt);
    }
    stats_tick();
}

extern "C" void hook_0200ECD4(Cpu* c) {
    if (t_dt < 1.0f) {
        c->f[2].ps0 = (float)frac(c->f[2].ps0);
        c->f[3].ps0 = (float)(c->f[3].ps0 * t_dt);
        c->f[4].ps0 = (float)(c->f[4].ps0 * t_dt);
    }
    f_0200ECD4_orig(c);
}

extern "C" void hook_0200ED84(Cpu* c) {
    if (t_dt < 1.0f) {
        c->f[2].ps0 = (float)frac(c->f[2].ps0);
        c->f[3].ps0 = (float)(c->f[3].ps0 * t_dt);
    }
    f_0200ED84_orig(c);
}

extern "C" void hook_0200EDC8(Cpu* c) {
    if (t_dt < 1.0f) {
        c->f[1].ps0 = (float)frac(c->f[1].ps0);
        c->f[2].ps0 = (float)(c->f[2].ps0 * t_dt);
    }
    f_0200EDC8_orig(c);
}

extern "C" void hook_0200EE00(Cpu* c) {
    if (t_dt < 1.0f) {
        c->f[1].ps0 = (float)frac(c->f[1].ps0);
        c->f[2].ps0 = (float)(c->f[2].ps0 * t_dt);
        c->f[3].ps0 = (float)(c->f[3].ps0 * t_dt);
    }
    f_0200EE00_orig(c);
}
extern "C" void hook_0200EF78(Cpu* c) {
    if (t_dt < 1.0f) {
        c->f[1].ps0 = (float)frac(c->f[1].ps0);
        c->f[2].ps0 = (float)(c->f[2].ps0 * t_dt);
        c->f[3].ps0 = (float)(c->f[3].ps0 * t_dt);
    }
    f_0200EF78_orig(c);
}

extern "C" void hook_0200F164(Cpu* c) {
    if (t_dt < 1.0f) {
        c->f[1].ps0 = (float)frac(c->f[1].ps0);
        c->f[2].ps0 = (float)(c->f[2].ps0 * t_dt);
    }
    f_0200F164_orig(c);
}
extern "C" void hook_0200F268(Cpu* c) {
    if (t_dt < 1.0f) {
        c->f[1].ps0 = (float)frac(c->f[1].ps0);
        c->f[2].ps0 = (float)(c->f[2].ps0 * t_dt);
    }
    f_0200F268_orig(c);
}

static int32_t div_step(int32_t diff, int32_t scale) {
    if (scale == 0) return 0;
    if (t_dt >= 1.0f || scale == 1 || scale == -1) return diff / scale;
    double s = 1.0 / (double)scale;
    double f = s > 0 ? frac(s) : -frac(-s);
    return (int32_t)((double)diff * f);
}

extern "C" void hook_0200F378(Cpu* c) {
    if (t_dt >= 1.0f) return f_0200F378_orig(c);
    uint32_t p = c->r[3];
    int16_t v = (int16_t)ld16(p), target = (int16_t)c->r[4], scale = (int16_t)c->r[5];
    int16_t maxStep = (int16_t)split((int16_t)c->r[6]), minStep = (int16_t)split((int16_t)c->r[7]);
    int16_t diff = (int16_t)(target - v);
    if (v != target) {
        int16_t step = (int16_t)div_step(diff, scale);
        if (step > minStep || step < -minStep) {
            if (step > maxStep) step = maxStep;
            if (step < -maxStep) step = (int16_t)-maxStep;
            v = (int16_t)(v + step);
        } else if (0 <= diff) {
            v = (int16_t)(v + minStep);
            if (0 >= (int16_t)(target - v)) v = target;
        } else {
            v = (int16_t)(v - minStep);
            if (0 <= (int16_t)(target - v)) v = target;
        }
        st16(p, (uint16_t)v);
    }
    c->r[3] = (uint32_t)(int32_t)(int16_t)(target - v);
}

extern "C" void hook_0200F428(Cpu* c) {
    if (t_dt >= 1.0f) return f_0200F428_orig(c);
    uint32_t p = c->r[3];
    int16_t v = (int16_t)ld16(p), target = (int16_t)c->r[4], scale = (int16_t)c->r[5];
    int16_t maxStep = (int16_t)split((int16_t)c->r[6]);
    int16_t step = (int16_t)div_step((int16_t)(target - v), scale);
    if (step > maxStep) v = (int16_t)(v + maxStep);
    else if (step < -maxStep) v = (int16_t)(v - maxStep);
    else v = (int16_t)(v + step);
    st16(p, (uint16_t)v);
}

extern "C" void hook_0200F474(Cpu* c) {
    if (t_dt >= 1.0f) return f_0200F474_orig(c);
    uint32_t p = c->r[3];
    int32_t v = (int32_t)ld32(p), target = (int32_t)c->r[4], scale = (int32_t)c->r[5];
    int32_t maxStep = split((int32_t)c->r[6]), minStep = split((int32_t)c->r[7]);
    int32_t diff = target - v;
    if (v != target) {
        int32_t step = div_step(diff, scale);
        if (step > minStep || step < -minStep) {
            if (step > maxStep) step = maxStep;
            if (step < -maxStep) step = -maxStep;
            v += step;
        } else if (0 <= diff) {
            v += minStep;
            if (0 >= target - v) v = target;
        } else {
            v -= minStep;
            if (0 <= target - v) v = target;
        }
        st32(p, (uint32_t)v);
    }
    c->r[3] = (uint32_t)diff;
}

extern "C" void hook_0200F4FC(Cpu* c) {
    if (t_dt < 1.0f && (c->r[5] & 0xFF)) {
        int32_t s = split((int32_t)(c->r[5] & 0xFF));
        if (s == 0) {
            c->r[3] = ld8(c->r[3]) == (c->r[4] & 0xFF);
            return;
        }
        c->r[5] = (uint32_t)s;
    }
    f_0200F4FC_orig(c);
}
extern "C" void hook_0200F564(Cpu* c) {
    if (t_dt < 1.0f && (int16_t)c->r[5]) {
        int32_t s = split((int16_t)c->r[5]);
        if (s == 0) {
            c->r[3] = (int16_t)ld16(c->r[3]) == (int16_t)c->r[4];
            return;
        }
        c->r[5] = (uint32_t)s;
    }
    f_0200F564_orig(c);
}
extern "C" void hook_0200F5C8(Cpu* c) {
    if (t_dt < 1.0f) c->f[2].ps0 = (float)(c->f[2].ps0 * t_dt);
    f_0200F5C8_orig(c);
}
extern "C" void hook_0200F62C(Cpu* c) {
    if (t_dt < 1.0f) c->f[1].ps0 = (float)(c->f[1].ps0 * t_dt);
    f_0200F62C_orig(c);
}
extern "C" void hook_0200F764(Cpu* c) {
    if (t_dt < 1.0f) c->f[1].ps0 = (float)(c->f[1].ps0 * t_dt);
    f_0200F764_orig(c);
}
extern "C" void hook_0200F8D0(Cpu* c) {
    if (t_dt < 1.0f && (int16_t)c->r[5]) {
        int32_t s = split((int16_t)c->r[5]);
        if (s == 0) {
            c->r[3] = (int16_t)ld16(c->r[3]) == (int16_t)c->r[4];
            return;
        }
        c->r[5] = (uint32_t)s;
    }
    f_0200F8D0_orig(c);
}

namespace interp { bool fx_hold_anim(); }
namespace {
struct FcStep { uint64_t pass; uint32_t before, after, rate, range; };
std::unordered_map<uint32_t, FcStep> g_fc;
}
extern "C" void hook_027F2FC4(Cpu* c) {

    if (interp::fx_hold_anim()) return;
    if (t_dt >= 1.0f) return f_027F2FC4_orig(c);
    uint32_t fc = c->r[3];
    uint32_t rate = ld32(fc), frame = ld32(fc + 4), range = ld32(fc + 8);
    if (!half_pass()) {
        auto it = g_fc.find(fc);
        bool halved = false;
        if (it != g_fc.end()) {
            FcStep st = it->second;
            g_fc.erase(it);
            if (st.pass + 1 == g_pass) {
                halved = true;
                if (st.after == frame && st.rate == rate && st.range == range) {
                    st32(fc + 4, st.before);
                    f_027F2FC4_orig(c);
                    return;
                }
            }
        }

        if (!halved) return f_027F2FC4_orig(c);
    }
    float scaled = u32_as_f32(rate) * t_dt;
    st32(fc, f32_as_u32(scaled));
    f_027F2FC4_orig(c);
    if (ld32(fc) == f32_as_u32(scaled)) st32(fc, rate);
    if (half_pass()) {
        if (g_fc.size() > 4096) g_fc.clear();
        g_fc[fc] = FcStep{g_pass, frame, ld32(fc + 4), ld32(fc), range};
    }
}

extern "C" void f_027F2BF8_orig(Cpu* c);
extern "C" void hook_027F2BF8(Cpu* c) {
    if (t_dt >= 1.0f || !half_pass()) return f_027F2BF8_orig(c);
    c->r[3] = 0;
}

extern "C" void hook_025E3EC8(Cpu* c) {
    if (t_dt >= 1.0f) return f_025E3EC8_orig(c);
    uint32_t p = c->r[3];
    float cnt = u32_as_f32(ld32(p + 4));
    if (!(cnt > 0.0f)) return;
    cnt -= t_dt;
    if (cnt <= 0.0f) {
        cnt = 0.0f;
        st32(p + 8, f32_as_u32(0.0f));
        st32(p + 0xC, f32_as_u32(0.0f));
    }
    st32(p + 4, f32_as_u32(cnt));
    float f10 = u32_as_f32(ld32(p + 0x10));
    st32(p + 0x14, f32_as_u32(f10));
    float n10 = cnt * u32_as_f32(ld32(p + 8));
    st32(p + 0x10, f32_as_u32(n10));
    st32(p + 0xC, f32_as_u32(f10 > 0.0f ? 1.0f - (f10 - n10) / f10 : 0.0f));
}

static thread_local uint32_t t_dv_actor = 0;
static thread_local float t_dv = 0.0f;
extern "C" void hook_025D67A8(Cpu* c) {
    if (t_dt >= 1.0f) return f_025D67A8_orig(c);
    uint32_t a = c->r[3];
    float vy = u32_as_f32(ld32(a + kSpeed + 4));
    float g = u32_as_f32(ld32(a + kGravity));
    f_025D67A8_orig(c);
    float full = u32_as_f32(ld32(a + kSpeed + 4));
    float maxf = u32_as_f32(ld32(a + kMaxFall));
    float half = vy + g * t_dt;
    if (half < maxf) half = maxf;
    if (full != vy + g && full != maxf) return;
    st32(a + kSpeed + 4, f32_as_u32(half));
    t_dv_actor = a;
    t_dv = half - vy;
}

extern "C" void hook_025D6800(Cpu* c) {
    if (t_dt >= 1.0f) return f_025D6800_orig(c);
    uint32_t a = c->r[3], mv = c->r[4];

    float corr = t_dv_actor == a ? t_dv * (1.0f - t_dt) * 0.5f : 0.0f;
    t_dv_actor = 0;
    for (int i = 0; i < 3; i++) {
        float p = u32_as_f32(ld32(a + kPos + 4 * i)) + u32_as_f32(ld32(a + kSpeed + 4 * i)) * t_dt + (i == 1 ? corr : 0.0f);
        if (mv) p += u32_as_f32(ld32(mv + 4 * i));
        st32(a + kPos + 4 * i, f32_as_u32(p));
    }
}

static uint32_t g_draw_stack[8];
static int g_draw_depth = 0;
static int g_force_draw_60 = 0;
void true60::force_draw_60(bool on) { g_force_draw_60 += on ? 1 : -1; }
bool true60::drawing_60() {
    if (enabled() && g_force_draw_60 > 0) return true;
    return enabled() && g_draw_depth > 0 && g_draw_depth <= 8 && runs_60(g_draw_stack[g_draw_depth - 1]);
}

extern "C" void hook_025DF904(Cpu* c) {
    uint32_t proc = c->r[3];
    if (g_draw_depth < 8) g_draw_stack[g_draw_depth] = proc;
    g_draw_depth++;
    uint32_t wv = watch_val();
    static const uint32_t dlp = getenv("NSMBU_DRAWLOG_PROC") ? (uint32_t)strtoul(getenv("NSMBU_DRAWLOG_PROC"), nullptr, 16) : 0;
    if (dlp && proc == dlp) {
        static const uint32_t off = getenv("NSMBU_DRAWLOG_OFF") ? (uint32_t)strtoul(getenv("NSMBU_DRAWLOG_OFF"), nullptr, 16) : 0;
        LOG("[drawlog] step %llu %s depth %d word %08X exec %08X", (unsigned long long)interp::logic_steps(), half_pass() ? "half" : "full", g_draw_depth, ld32(proc + off), actor_execute_fn(proc));
        static FILE* df = getenv("NSMBU_DRAWLOG_DUMP") ? fopen(getenv("NSMBU_DRAWLOG_DUMP"), "wb") : nullptr;
        if (df && !half_pass()) {
            static uint32_t rlo = getenv("NSMBU_DRAWLOG_RANGE") ? (uint32_t)strtoul(getenv("NSMBU_DRAWLOG_RANGE"), nullptr, 16) : GD(0x10475A68);
            static uint32_t rsz = getenv("NSMBU_DRAWLOG_RANGE") && strchr(getenv("NSMBU_DRAWLOG_RANGE"), ':') ? (uint32_t)strtoul(strchr(getenv("NSMBU_DRAWLOG_RANGE"), ':') + 1, nullptr, 16) : 0x2000;
            uint64_t step = interp::logic_steps(); uint32_t full = 1, size = 0x400 + rsz; float dtv = 1.0f;
            fwrite("ADMP", 1, 4, df); fwrite(&step, 8, 1, df); fwrite(&full, 4, 1, df); fwrite(&dtv, 4, 1, df);
            fwrite(&proc, 4, 1, df); fwrite(&size, 4, 1, df); fwrite(mem::ptr(proc), 1, 0x400, df); fwrite(mem::ptr(rlo), 1, rsz, df);
            fflush(df);
        }
    }
    static const bool keep = getenv("NSMBU_TRUE60_DRAW_KEEP") != nullptr;
    static std::vector<uint32_t> before;
    uint32_t size = 0, cull_mtx = 0;
    uint8_t cull_before[0x30];
    if (enabled() && half_pass() && !keep && g_draw_depth >= 2 && !runs_60(proc) && actor_execute_fn(proc)) {
        size = ld32(proc - 8) & ~3u;
        if (size < 0x100 || size > 0x20000) size = 0;
        if (size) before.assign((uint32_t*)ppc_ptr(proc), (uint32_t*)ppc_ptr(proc + size));

        cull_mtx = ld32(proc + 0x348);
        if (cull_mtx >= mem::kMem2Start && cull_mtx < mem::kMem2End && (cull_mtx & 3) == 0)
            memcpy(cull_before, ppc_ptr(cull_mtx), 0x30);
        else
            cull_mtx = 0;
    }
    static const bool envlog = getenv("NSMBU_T60_ENVLOG") != nullptr;
    static std::vector<uint32_t> env_before;
    bool envdbg = envlog && proc == g_link && half_pass();
    if (envdbg) env_before.assign((uint32_t*)ppc_ptr(GD(0x10475A68)), (uint32_t*)ppc_ptr(GD(0x10475A68) + 0x2000));

    static const bool dwlog = getenv("NSMBU_T60_DRAWWRITE") != nullptr;
    static std::vector<uint32_t> dw_before;
    bool dw = dwlog && half_pass() && g_draw_depth >= 2;
    if (dw) dw_before.assign((uint32_t*)ppc_ptr(GD(0x1046F0B0)), (uint32_t*)ppc_ptr(GD(0x104C3000)));
    f_025DF904_orig(c);
    if (dw) {
        static std::unordered_map<uint32_t, int> cnt;
        static std::unordered_map<uint32_t, uint32_t> who;
        static int n = 0;
        const uint32_t* cur = (const uint32_t*)ppc_ptr(GD(0x1046F0B0));
        for (size_t i = 0; i < dw_before.size(); i++)
            if (cur[i] != dw_before[i]) { uint32_t a = GD(0x1046F0B0) + 4 * (uint32_t)i; cnt[a]++; who[a] = actor_execute_fn(proc); }
        if (++n % 3000 == 0) {
            std::vector<std::pair<uint32_t, int>> v(cnt.begin(), cnt.end());
            std::sort(v.begin(), v.end());
            std::string o;
            for (auto& [a, k] : v) { char t[40]; snprintf(t, sizeof t, " %08X:%d:%08X", a, k, who[a]); o += t; }
            LOG("[drawwrite]%s", o.c_str());
        }
    }
    if (envdbg) {
        std::string o;
        const uint32_t* cur = (const uint32_t*)ppc_ptr(GD(0x10475A68));
        for (size_t i = 0; i < env_before.size(); i++)
            if (cur[i] != env_before[i]) { char t[16]; snprintf(t, sizeof t, " +%zX", 4 * i); o += t; }
        static int n = 0;
        if (n++ < 20) LOG("[envlog] Link draw changed g_env_light:%s", o.c_str());
    }
    if (size) {
        uint32_t* cur = (uint32_t*)ppc_ptr(proc);

        auto entered_packet = [&](uint32_t i) {
            if (i * 4 < 0x94) return false;
            uint32_t slot = __builtin_bswap32(cur[i]);
            if (slot < mem::kMem2Start || slot >= mem::kMem2End || (slot & 3)) return false;
            uint32_t pkt = proc + 4 * i - 0x94;
            uint32_t q = ld32(slot);
            for (int n = 0; q && n < 4096; n++) {
                if (q == pkt) return true;
                if (q < mem::kMem2Start || q >= mem::kMem2End || (q & 3)) return false;
                q = ld32(q + 0x10);
            }
            return false;
        };
        static std::vector<uint32_t> keep;
        keep.clear();
        for (uint32_t i = 0x94 / 4; i < size / 4; i++)
            if (cur[i] != before[i] && entered_packet(i)) {
                keep.push_back(i);
                keep.push_back(i - (0x94 - 0x10) / 4);
            }
        for (uint32_t i = 0; i < size / 4; i++)
            if (cur[i] != before[i] && std::find(keep.begin(), keep.end(), i) == keep.end()) cur[i] = before[i];
        if (cull_mtx) memcpy(ppc_ptr(cull_mtx), cull_before, 0x30);
    }
    watch_link("draw", proc, wv);
    g_draw_depth--;
}

extern "C" void hook_025028B8(Cpu* c) {
    g_follow_called = true;
    f_025028B8_orig(c);
}

namespace interp { bool enabled(); }

static bool hold_world() {
    static const bool every = getenv("NSMBU_WORLD_EVERY_PASS") != nullptr;
    return interp::enabled() && interp::hold_pass() && !every;
}
extern "C" void site_025B00B0(Cpu* c) {
    if (hold_world()) c->r[3] = 1;
}
extern "C" void hook_025CB6D4(Cpu* c) {
    if (c->lr == GC(0x025B01F0u) && hold_world()) return;
    f_025CB6D4_orig(c);
}

extern "C" void hook_024EF968(Cpu* c) {
    uint32_t pos = c->r[6];
    bool link = g_link && pos == g_link + kPos;
    float before[3] = {0, 0, 0};
    if (link)
        for (int i = 0; i < 3; i++) before[i] = u32_as_f32(ld32(pos + 4 * i));
    f_024EF968_orig(c);
    if (link)
        for (int i = 0; i < 3; i++)
            if (u32_as_f32(ld32(pos + 4 * i)) != before[i]) {
                if (g_link_ride == 0 && enabled()) LOG("[true60] Link rides moving collision: 30 Hz");
                g_link_ride = 8;
                break;
            }
}

extern "C" void f_02018D40_orig(Cpu* c);
extern "C" void hook_02018D40(Cpu* c) {
    if (g_cam_step) {
        uint32_t p = c->r[4];
        if (std::isnan(ldf32(p)) || std::isnan(ldf32(p + 4)) || std::isnan(ldf32(p + 8))) {
            if (!g_cam_nan) g_cam_nan_lr = c->lr;
            g_cam_nan = true;
            return;
        }
    }
    f_02018D40_orig(c);
}

void true60_nan_probe(uint32_t addr) {
    static uint32_t hist[64];
    static uint32_t n = 0;
    static int reported = 0;
    static uint64_t step_seen = ~0ull;
    if (!g_cam_step || reported >= 3) return;
    uint32_t cam = g_camera + kDCamera;
    if (step_seen != g_pass) step_seen = g_pass, n = 0;
    Cpu* c = threads::current();
    hist[n++ & 63] = addr;
    if (n < 64 || (n & 63) == 0) {}
    bool nan = false;
    for (uint32_t o = 0x10; o <= 0x60 && !nan; o += 4) nan = std::isnan(ldf32(cam + o));
    static bool was_nan = false;
    if (nan && !was_nan) {
        reported++;
        LOG("[nanprobe] pass %llu dt %.2f: camera state NaN at entry of %08X (lr %08X); last entries:", (unsigned long long)g_pass, t_dt, addr,
            c ? c->lr : 0);
        for (uint32_t i = n > 40 ? n - 40 : 0; i < n; i++) LOG("[nanprobe]   %08X", hist[i & 63]);
        for (uint32_t o = 0x10; o <= 0x60; o += 4) if (std::isnan(ldf32(cam + o))) LOG("[nanprobe]   NaN at dCamera+0x%02X", o);
    }
    was_nan = nan;
}

extern "C" void f_02593B10_orig(Cpu* c);
extern "C" void hook_02593B10(Cpu* c) {
    static const bool every = getenv("NSMBU_HUD_EVERY_PASS") != nullptr;
    if (enabled() && half_pass() && !every) return;
    f_02593B10_orig(c);
}
