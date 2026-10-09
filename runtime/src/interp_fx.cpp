

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

#include "guest_addr.h"
#include "interp_pacing.h"
#include "runtime.h"
#include "true60.h"

namespace interp {
bool enabled();
bool hold_pass();
bool logic_pass();
bool blend_draw();
bool record_pass();
float pass_t();
bool in_execute();
uint64_t hold_pass_count();
uint64_t pass_count();
uint64_t logic_steps();
}

extern "C" {
void f_0282167C_orig(Cpu* c);
void f_025AF8A0_orig(Cpu* c);
void f_027DF40C_orig(Cpu* c);
void f_027F2FC4_orig(Cpu* c);
void f_0246C5E8_orig(Cpu* c);
void f_0246C7C8_orig(Cpu* c);
void f_028E93CC_orig(Cpu* c);
void f_0246BD4C_orig(Cpu* c);
void f_025A8148_orig(Cpu* c);
void f_0254C6C4_orig(Cpu* c);
void f_025C9948_orig(Cpu* c);
void f_02548370_orig(Cpu* c);
void f_0256A448_orig(Cpu* c);
void f_025D0994_orig(Cpu* c);
void f_02566B88_orig(Cpu* c);
void f_02568BD4_orig(Cpu* c);
void f_02569408_orig(Cpu* c);
void f_02567F68_orig(Cpu* c);
void f_0256BB6C_orig(Cpu* c);
void f_0256CA54_orig(Cpu* c);
void f_0256DDF8_orig(Cpu* c);
void f_0256A388_orig(Cpu* c);
void f_0251D864_orig(Cpu* c);
void f_0281FE40_orig(Cpu* c);
void f_024EC1C8_orig(Cpu* c);
}

namespace {
uint32_t mask() {
    static const uint32_t m = getenv("NSMBU_INTERP_FX") ? (uint32_t)strtoul(getenv("NSMBU_INTERP_FX"), nullptr, 0) : 0xFF;
    return m;
}
bool on(uint32_t bit) { return interp::enabled() && (mask() & bit); }
bool hold_back() { return interp::hold_pass() && on(8); }

bool halfway() { return interp::blend_draw() && !interp::in_execute(); }

using interp::pacing::lerp_f;
using interp::pacing::lerp_s16;
using interp::pacing::lerp_u8;

int trace_left(int part) {
    static int left[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    if (left[part] < 0) left[part] = getenv("NSMBU_INTERP_FX_TRACE") ? atoi(getenv("NSMBU_INTERP_FX_TRACE")) : 0;
    return left[part] > 0 ? left[part]-- : 0;
}

constexpr uint32_t kMgrGroups = 0x50, kEmtrPtcls = 0x1AC, kEmtrChildren = 0x1B8;
constexpr uint32_t kPtclAge = 0x78, kPtclVel = 0x34;
const uint32_t kEmtrInfo = GD(0x104B5730), kInfoCenter = 0xE0, kInfoScale = 0xEC;
constexpr int kPW = 12;
constexpr uint32_t kPtclWord[kPW] = {0x28, 0x2C, 0x30, 0x9C, 0xA0, 0x8C, 0x90, 0x94, 0xAC, 0xB8, 0xBC, 0xC0};
enum { kWPos = 0, kWSize = 3, kWAxis = 5, kWAlpha = 8, kWPrm = 9, kWEnv = 10, kWRot = 11 };

struct PtclState {
    uint32_t w[kPW];
    float age;
};
struct Applied {
    uint32_t ptcl;
    uint32_t w[kPW];
    uint32_t a[kPW];
    float born[3];
    bool is_born, extras;
};
std::unordered_map<uint32_t, PtclState> g_ptcl_before;
std::vector<Applied> g_ptcl_applied;
struct { uint32_t blended = 0, fresh = 0, born = 0; } g_pstats;

struct EmtrRec {
    float prev[3], now[3], scale[3];
    uint64_t step = 0, prev_step = 0;
};
std::unordered_map<uint32_t, EmtrRec> g_emtr;

float wf(uint32_t w) { return u32_as_f32(__builtin_bswap32(w)); }
uint32_t fw(float f) { return __builtin_bswap32(f32_as_u32(f)); }

PtclState read_ptcl(uint32_t p) {
    PtclState s;
    for (int i = 0; i < kPW; i++) s.w[i] = *(const uint32_t*)ppc_ptr(p + kPtclWord[i]);
    s.age = (float)ldf32(p + kPtclAge);
    return s;
}
void write_ptcl(uint32_t p, const uint32_t* w) {
    for (int i = 0; i < kPW; i++) *(uint32_t*)ppc_ptr(p + kPtclWord[i]) = w[i];
}

void mid_ptcl(const uint32_t* a, const uint32_t* b, uint32_t* m, bool extras, float t) {
    for (int i = 0; i < kPW; i++) m[i] = b[i];
    for (int i = kWPos; i < kWSize + 2; i++) m[i] = fw(lerp_f(wf(a[i]), wf(b[i]), t));
    if (!extras) return;
    for (int i = kWAxis; i <= kWAlpha; i++) m[i] = fw(lerp_f(wf(a[i]), wf(b[i]), t));
    for (int i : {kWPrm, kWEnv}) {
        const uint8_t* x = (const uint8_t*)&a[i];
        const uint8_t* y = (const uint8_t*)&b[i];
        uint8_t* o = (uint8_t*)&m[i];
        for (int k = 0; k < 4; k++) o[k] = lerp_u8(x[k], y[k], t);
    }

    uint16_t ra = (uint16_t)(((const uint8_t*)&a[kWRot])[0] << 8 | ((const uint8_t*)&a[kWRot])[1]);
    uint16_t rb = (uint16_t)(((const uint8_t*)&b[kWRot])[0] << 8 | ((const uint8_t*)&b[kWRot])[1]);
    uint16_t rm = (uint16_t)lerp_s16((int16_t)ra, (int16_t)rb, t);
    ((uint8_t*)&m[kWRot])[0] = (uint8_t)(rm >> 8);
    ((uint8_t*)&m[kWRot])[1] = (uint8_t)rm;
}

template <class F>
void for_each_ptcl(uint32_t mgr, uint32_t group, F&& f) {
    int guard = 0;
    for (uint32_t el = ld32(mgr + kMgrGroups + 12 * group); el && guard < 100000; el = ld32(el + 0xC), guard++) {
        uint32_t emtr = ld32(el);
        if (!emtr) continue;
        for (uint32_t list : {kEmtrPtcls, kEmtrChildren})
            for (uint32_t pl = ld32(emtr + list); pl && guard < 100000; pl = ld32(pl + 0xC), guard++)
                if (uint32_t p = ld32(pl)) f(p, emtr);
    }
}

void ptcl_restore() {
    for (const Applied& a : g_ptcl_applied) write_ptcl(a.ptcl, a.w);
    g_ptcl_applied.clear();
}

void ptcl_mid(const Applied& ap, float t, uint32_t* mid) {
    if (ap.is_born) {
        memcpy(mid, ap.w, sizeof ap.w);
        for (int i = 0; i < 3; i++) mid[kWPos + i] = fw(wf(ap.w[kWPos + i]) - (1 - t) * ap.born[i]);
    } else {
        mid_ptcl(ap.a, ap.w, mid, ap.extras, t);
    }
}

std::vector<Applied> g_ptcl_keep;
void ptcl_reapply(float t) {
    for (const Applied& ap : g_ptcl_keep) {
        uint32_t mid[kPW];
        ptcl_mid(ap, t, mid);
        write_ptcl(ap.ptcl, mid);
    }
    g_ptcl_applied.swap(g_ptcl_keep);
    g_ptcl_keep.clear();
}

struct PtclTrace {
    double step = 0, frac = 0;
    int n = 0;
    void add(const PtclState& a, const PtclState& b, const uint32_t* mid) {
        double d[3], m[3], dd = 0, dm = 0;
        for (int i = 0; i < 3; i++) {
            d[i] = wf(b.w[i]) - wf(a.w[i]);
            m[i] = wf(mid[i]) - wf(a.w[i]);
            dd += d[i] * d[i];
            dm += d[i] * m[i];
        }
        float aa = wf(a.w[kWAlpha]), ab = wf(b.w[kWAlpha]);
        if (aa != ab) afrac += (wf(mid[kWAlpha]) - aa) / (ab - aa), an++;
        auto rot = [](const uint32_t* w) { return (int)(((const uint8_t*)&w[kWRot])[0] << 8 | ((const uint8_t*)&w[kWRot])[1]); };
        int16_t rd = (int16_t)(rot(b.w) - rot(a.w));
        if (rd) rfrac += (double)(int16_t)(rot(mid) - rot(a.w)) / rd, rn++;
        int ca = ((const uint8_t*)&a.w[kWPrm])[0], cb = ((const uint8_t*)&b.w[kWPrm])[0];
        if (ca != cb) cfrac += (double)(((const uint8_t*)&mid[kWPrm])[0] - ca) / (cb - ca), cn++;
        if (dd < 0.01) return;
        step += std::sqrt(dd);
        frac += dm / dd;
        n++;
    }
    double afrac = 0, rfrac = 0, cfrac = 0;
    int an = 0, rn = 0, cn = 0;
    void log(uint32_t group) {
        if (n || an || rn || cn)
            LOG("[interp-fx] particles group %u: %d moving, mean step %.2f units, halfway at %.3f of the step; alpha %.3f (%d), rotation %.3f (%d), "
                "colour %.3f (%d)",
                group, n, n ? step / n : 0.0, n ? frac / n : 0.0, an ? afrac / an : 0.0, an, rn ? rfrac / rn : 0.0, rn, cn ? cfrac / cn : 0.0, cn);
    }
};

const uint32_t kLoopFn = GC(0x027DA9E8);
struct AnmRec {
    uint32_t ts = 0;
    float frame = 0;
    uint64_t pass = 0;
};
std::unordered_map<uint32_t, AnmRec> g_anm;
struct { uint32_t blended = 0, wrapped = 0, jumped = 0; } g_astats;

float anm_mid(float a, float b, float start, float end, bool loop, float t) {
    float d = b - a;
    float len = end - start;
    if (loop && len > 0) {
        if (d < -0.5f * len) d += len, g_astats.wrapped++;
        else if (d > 0.5f * len) d -= len, g_astats.wrapped++;
    }
    if (!(std::fabs(d) <= 4.0f)) {
        g_astats.jumped++;
        return b;
    }
    float m = a + t * d;
    if (loop && len > 0) {
        if (m >= end) m -= len;
        else if (m < start) m += len;
    }
    return m;
}

constexpr int kSeaCells = 65 * 65;
constexpr uint32_t kSeaHeights = 0x20C, kSeaMinX = 0x1FC, kSeaMinZ = 0x200, kSeaScroll = 0x22C;
struct SeaRec {
    uint32_t packet = 0;
    uint64_t pass = 0;
    float min_x = 0, min_z = 0;
    std::vector<uint32_t> h;
} g_sea;
bool g_sea_scroll_half = false;
}

namespace {

struct Temp {
    uint32_t addr, exact, prev;
    int8_t kind;
    int8_t shift;
};
enum { kF32, kS16 };
std::vector<Temp> g_temp, g_temp_keep;

uint32_t temp_value(const Temp& e, float t) {
    if (e.kind == kF32) return f32_as_u32(lerp_f(u32_as_f32(e.prev), u32_as_f32(e.exact), t));
    int16_t b = (int16_t)(e.exact >> e.shift);
    uint16_t m = (uint16_t)lerp_s16((int16_t)e.prev, b, t);
    return (ld32(e.addr) & ~(0xFFFFu << e.shift)) | ((uint32_t)m << e.shift);
}

void temp_blend_f32(uint32_t a, uint32_t prev, float t) {
    Temp e{a, ld32(a), prev, kF32, 0};
    g_temp.push_back(e);
    st32(a, temp_value(e, t));
}

void temp_blend_s16(uint32_t a, int16_t prev, float t) {
    uint32_t w = a & ~3u;
    Temp e{w, ld32(w), (uint32_t)(uint16_t)prev, kS16, (int8_t)((a & 2) ? 0 : 16)};
    g_temp.push_back(e);
    st32(w, temp_value(e, t));
}
void temp_restore() {
    for (auto it = g_temp.rbegin(); it != g_temp.rend(); ++it) st32(it->addr, it->exact);
    g_temp_keep.swap(g_temp);
    g_temp.clear();
}
void temp_reapply(float t) {
    for (const Temp& e : g_temp_keep) st32(e.addr, temp_value(e, t));
    g_temp.swap(g_temp_keep);
    g_temp_keep.clear();
}
bool plausible(float v) { return v == 0.0f || (std::fabs(v) > 1e-12f && std::fabs(v) < 1e9f); }

const uint32_t kCounterTimer = GD(0x101FF560);
struct SwayKind {
    uint32_t base, stride;
    int fields;
    uint32_t field[2];
};
constexpr SwayKind kSway[3] = {{0x18F0C, 0x38, 1, {4, 0}}, {0x2A9C, 0x84, 2, {4, 6}}, {0x35BC, 0x38, 1, {4, 0}}};
struct SwayRec {
    uint32_t pkt = 0;
    uint64_t step = 0;
    int16_t v[8][2];
} g_sway[3];

void sway_calc(Cpu* c, int k, void (*orig)(Cpu*)) {
    const SwayKind& sk = kSway[k];
    uint32_t pk = c->r[3];
    orig(c);
    if (!on(16) || !halfway()) return;
    SwayRec& r = g_sway[k];
    bool valid = r.pkt == pk && r.step + 1 == interp::hold_pass_count();
    static int tr = trace_left(4);
    for (int i = 0; i < 8; i++)
        for (int f = 0; f < sk.fields; f++) {
            uint32_t a = pk + sk.base + sk.stride * i + sk.field[f];
            int16_t cur = (int16_t)ld16(a);
            if (valid) {
                temp_blend_s16(a, r.v[i][f], interp::pass_t());
                if (tr && i == 0 && f == 0) {
                    tr--;
                    LOG("[interp-fx] %s sway slot 0: %d -> %d, drawn %d", k == 0 ? "grass" : k == 1 ? "tree" : "flower", r.v[i][f], cur, (int16_t)ld16(a));
                }
            }
            r.v[i][f] = cur;
        }
    r.pkt = pk;
    r.step = interp::hold_pass_count();
}

constexpr uint32_t kWoodAnm = 0x1C8D8, kWoodAnmStride = 0x8C;
constexpr int kWoodAnms = 72, kWoodWords = 24;
struct WoodRec {
    uint32_t pkt = 0;
    uint64_t step = 0;
    std::vector<uint32_t> w;
} g_wood;

const uint32_t kEnvLight = GD(0x10475A68);
struct KankyoKind {
    const char* name;
    uint32_t env_off, base, stride;
    int count;
    int status;
    float max_step;
};
constexpr KankyoKind kKankyo[] = {
    {"rain", 0xA44, 0xA0, 0x38, 250, 0, 400.0f},
    {"snow", 0xA50, 0x9C, 0x38, 250, 0, 400.0f},
    {"housi", 0xA78, 0x9C, 0x50, 300, 0, 400.0f},
    {"moya", 0xA84, 0xA0, 0x4C, 100, 0, 600.0f},
    {"poison", 0xA6C, 0x98, 0x30, 1000, 0, 400.0f},
    {"vrkumo", 0xA94, 0xA4, 0x2C, 100, 0, 5000.0f},
    {"star", 0xA60, 0x9C, 0x34, 1, -1, 1e9f},
};
enum { kRain, kSnow, kHousi, kMoya, kPoison, kVrkumo, kStar };
struct KankyoTrace { int moving = 0, words = 0; double frac = 0; };

void kankyo_move(Cpu* c, int k, void (*orig)(Cpu*)) {
    const KankyoKind& kk = kKankyo[k];
    uint32_t pk = ld32(kEnvLight + kk.env_off);
    if (!on(32) || !interp::logic_pass() || pk < 0x10000000 || pk >= 0x50000000) {
        orig(c);
        return;
    }
    const uint32_t bytes = kk.stride * kk.count;
    std::vector<uint32_t> before((const uint32_t*)ppc_ptr(pk + kk.base), (const uint32_t*)ppc_ptr(pk + kk.base) + bytes / 4);
    orig(c);
    if (ld32(kEnvLight + kk.env_off) != pk) return;
    static int tr[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    if (tr[k] < 0) tr[k] = getenv("NSMBU_INTERP_FX_TRACE") ? atoi(getenv("NSMBU_INTERP_FX_TRACE")) : 0;
    KankyoTrace t;
    const uint32_t* now = (const uint32_t*)ppc_ptr(pk + kk.base);
    for (int i = 0; i < kk.count; i++) {
        uint32_t e = kk.stride * i;
        const uint32_t* a = &before[e / 4];
        const uint32_t* b = now + e / 4;
        if (kk.status >= 0 && ((const uint8_t*)a)[kk.status] != ((const uint8_t*)b)[kk.status]) continue;
        if (kk.status >= 0 && ((const uint8_t*)b)[kk.status] == 0) continue;
        float d2 = 0;
        for (int q = 1; q <= 3 && kk.status >= 0; q++) d2 += (wf(b[q]) - wf(a[q])) * (wf(b[q]) - wf(a[q]));
        if (!(d2 <= kk.max_step * kk.max_step)) continue;
        bool moved = false;
        for (uint32_t q = kk.status >= 0 ? 1 : 0; q < kk.stride / 4; q++) {
            if (a[q] == b[q]) continue;
            float x = wf(a[q]), y = wf(b[q]);
            if (!plausible(x) || !plausible(y) || std::fabs(y - x) > 1e5f) continue;
            temp_blend_f32(pk + kk.base + e + 4 * q, f32_as_u32(x), interp::pass_t());
            moved = true;
            if (tr[k] && q == 1 && x != y) t.frac += ((float)ldf32(pk + kk.base + e + 4 * q) - x) / (y - x), t.words++;
        }
        t.moving += moved;
    }
    if (tr[k] && t.moving) {
        tr[k]--;
        LOG("[interp-fx] %s sprites: %d moving, halfway at %.3f of the step (x, %d samples)", kk.name, t.moving, t.words ? t.frac / t.words : 0.0,
            t.words);
    }
}
}

namespace {

inline float hf(uint32_t w) { return u32_as_f32(w); }
inline uint32_t fh(float f) { return f32_as_u32(f); }

constexpr uint32_t kWaveEff = 0xA0, kWaveStride = 0x38;
constexpr int kWaves = 300;
constexpr int kWaveFields = 7;
constexpr uint32_t kWaveField[kWaveFields] = {0x0, 0x4, 0x8, 0x1C, 0x24, 0x28, 0x2C};
constexpr uint32_t kWaveSkew[2] = {0x4240, 0x4244};
struct WaveApplied {
    uint32_t pkt = 0;
    std::vector<uint32_t> exact, before;
    std::vector<uint8_t> blend;
    uint32_t skew[2], skew_before[2];
} g_wave;
uint32_t g_wave_keep = 0;

void wave_restore() {
    if (!g_wave.pkt) return;
    for (int i = 0; i < kWaves; i++)
        for (int f = 0; f < kWaveFields; f++)
            st32(g_wave.pkt + kWaveEff + kWaveStride * i + kWaveField[f], g_wave.exact[kWaveFields * i + f]);
    for (int k = 0; k < 2; k++) st32(g_wave.pkt + kWaveSkew[k], g_wave.skew[k]);
    g_wave_keep = g_wave.pkt;
    g_wave.pkt = 0;
}

void wave_apply(float t) {
    const uint32_t pk = g_wave.pkt;
    for (int k = 0; k < 2; k++) st32(pk + kWaveSkew[k], fh(lerp_f(hf(g_wave.skew_before[k]), hf(g_wave.skew[k]), t)));
    for (int i = 0; i < kWaves; i++) {
        if (!g_wave.blend[i]) continue;
        uint32_t e = pk + kWaveEff + kWaveStride * i;
        const uint32_t* a = &g_wave.before[kWaveFields * i];
        const uint32_t* w = &g_wave.exact[kWaveFields * i];
        for (int f = 0; f < kWaveFields; f++) st32(e + kWaveField[f], fh(lerp_f(hf(a[f]), hf(w[f]), t)));
    }
}
void wave_reapply(float t) {
    if (!g_wave_keep) return;
    g_wave.pkt = g_wave_keep;
    g_wave_keep = 0;
    wave_apply(t);
}
}

static uint32_t g_wave_pkt = 0;
static uint32_t g_wave_env_pkt = 0;
static uint64_t g_wave_pkt_seen = 0, g_passes = 0;
extern "C" void f_02555D0C(Cpu* c);
extern "C" void hook_0256A448(Cpu* c) {
    wave_restore();

    Cpu tmp = *c;
    f_02555D0C(&tmp);
    uint32_t pk = tmp.r[3] ? ld32(tmp.r[3] + 0xAA0) : 0;
    if (pk < 0x10000000 || pk >= 0x50000000) pk = 0;
    g_wave_env_pkt = pk;

    static int raw_left = getenv("NSMBU_WAVE_RAW") ? atoi(getenv("NSMBU_WAVE_RAW")) : 0;
    if (raw_left > 0 && pk && --raw_left == 0) {
        LOG("[waveraw] packet %08X env %08X count %d", pk, tmp.r[3], (int)(int16_t)ld16(tmp.r[3] + 0x9F8));
        for (int i = 0; i < 6; i++) {
            char b[400];
            int n = snprintf(b, sizeof b, "[waveraw] %2d:", i);
            for (uint32_t o = 0; o < kWaveStride; o += 4) {
                uint32_t v = ld32(pk + kWaveEff + kWaveStride * i + o);
                n += snprintf(b + n, sizeof b - n, " %02X=%08X(%g)", o, v, (double)hf(v));
            }
            LOG("%s", b);
        }
    }
    if (!on(2) || !interp::logic_pass() || !pk) {
        f_0256A448_orig(c);
        return;
    }
    std::vector<uint32_t> before(kWaveFields * kWaves), base(3 * kWaves), status(kWaves);
    uint32_t skew_before[2] = {ld32(pk + kWaveSkew[0]), ld32(pk + kWaveSkew[1])};
    for (int i = 0; i < kWaves; i++) {
        uint32_t e = pk + kWaveEff + kWaveStride * i;
        for (int f = 0; f < kWaveFields; f++) before[kWaveFields * i + f] = ld32(e + kWaveField[f]);
        for (int k = 0; k < 3; k++) base[3 * i + k] = ld32(e + 0xC + 4 * k);
        status[i] = ld8(e + 0x34);
    }
    f_0256A448_orig(c);
    g_wave_keep = 0;
    g_wave.pkt = pk;
    g_wave.exact.resize(kWaveFields * kWaves);
    g_wave.before = before;
    g_wave.blend.assign(kWaves, 0);
    for (int k = 0; k < 2; k++) {
        g_wave.skew[k] = ld32(pk + kWaveSkew[k]);
        g_wave.skew_before[k] = skew_before[k];
    }
    const float t = interp::pass_t();
    static int tr = trace_left(5);
    int moving = 0;
    double frac = 0;
    for (int i = 0; i < kWaves; i++) {
        uint32_t e = pk + kWaveEff + kWaveStride * i;
        uint32_t* w = &g_wave.exact[kWaveFields * i];
        for (int f = 0; f < kWaveFields; f++) w[f] = ld32(e + kWaveField[f]);
        const uint32_t* a = &before[kWaveFields * i];
        float d2 = 0;
        for (int k = 0; k < 3; k++) d2 += (hf(w[k]) - hf(a[k])) * (hf(w[k]) - hf(a[k]));

        bool respawned = ld32(e + 0xC) != base[3 * i] || ld32(e + 0x10) != base[3 * i + 1] || ld32(e + 0x14) != base[3 * i + 2] ||
                         ld8(e + 0x34) != status[i];
        if (respawned || !(d2 <= 300.0f * 300.0f) || !(std::fabs(hf(w[4]) - hf(a[4])) < 1000.0f)) continue;
        g_wave.blend[i] = 1;
        if (tr && d2 > 1e-4f) {
            moving++;
            frac += (lerp_f(hf(a[0]), hf(w[0]), t) - hf(a[0])) / (hf(w[0]) - hf(a[0]) != 0 ? (hf(w[0]) - hf(a[0])) : 1.0f);
        }
    }
    wave_apply(t);
    if (tr && moving) {
        tr--;
        LOG("[interp-fx] wave sprites: %d moving, drawn at %.3f of the step (x)", moving, frac / moving);
    }
}
extern "C" void f_02582500_orig(Cpu* c);

extern "C" void hook_02582500(Cpu* c) {
    g_wave_pkt = c->r[3];
    g_wave_pkt_seen = g_passes;

    static int dump_left = getenv("NSMBU_WAVE_DUMP") ? atoi(getenv("NSMBU_WAVE_DUMP")) : 0;
    static int pick[4] = {-1, -1, -1, -1};
    if (dump_left > 0 && interp::enabled()) {
        uint32_t pk = g_wave_env_pkt;
        if (!pk) goto no_dump;
        if (pick[0] < 0)
            for (int i = 0, n = 0; i < kWaves && n < 4; i++)
                if (hf(ld32(pk + kWaveEff + kWaveStride * i + 0x28)) > 0.3f) pick[n++] = i;
        dump_left--;
        for (int n = 0; n < 4 && pick[n] >= 0; n++) {
            uint32_t e = pk + kWaveEff + kWaveStride * pick[n];
            LOG("[wavedump] pass %llu %s crest %d pos %.1f %.1f %.1f base %.0f %.0f scale %.3f cspd %.4f counter %.4f sin %.3f alpha %.3f str %.3f st %d skew %.3f %.3f",
                (unsigned long long)g_passes, interp::hold_pass() ? "H" : "L", pick[n], hf(ld32(e)), hf(ld32(e + 4)), hf(ld32(e + 8)),
                hf(ld32(e + 0xC)), hf(ld32(e + 0x14)), hf(ld32(e + 0x1C)), hf(ld32(e + 0x20)), hf(ld32(e + 0x24)),
                std::sin(hf(ld32(e + 0x24))), hf(ld32(e + 0x28)), hf(ld32(e + 0x2C)), ld8(e + 0x34), hf(ld32(pk + 0x4240)),
                hf(ld32(pk + 0x4244)));
        }
    }
no_dump:
    static const bool stats = getenv("NSMBU_WAVE_STATS") != nullptr;
    if (stats) {
        static int n[2] = {0, 0}, vis = 0, total = 0;
        static uint64_t last_pass = 0;
        n[interp::hold_pass() ? 1 : 0]++;
        int v = 0;
        for (int i = 0; g_wave_env_pkt && i < kWaves; i++) v += hf(ld32(g_wave_env_pkt + kWaveEff + kWaveStride * i + 0x28)) > 0.01f;
        vis += v;
        total++;
        if (g_passes - last_pass >= 120) {
            LOG("[waves] last 120 passes: drawn %d times after logic passes, %d after hold passes; visible crests %.1f",
                n[0], n[1], total ? (double)vis / total : 0.0);
            n[0] = n[1] = vis = total = 0;
            last_pass = g_passes;
        }
    }
    f_02582500_orig(c);
}

extern "C" void hook_025A8148(Cpu* c) {
    if (hold_back()) {
        st32(kCounterTimer, ld32(kCounterTimer) - 1);
        return;
    }
    f_025A8148_orig(c);
}

extern "C" void hook_025D0994(Cpu* c) {
    if (hold_back()) return;
    uint32_t pk = c->r[3];
    f_025D0994_orig(c);
    if (!on(16) || !halfway()) return;
    bool valid = g_wood.pkt == pk && g_wood.step + 1 == interp::hold_pass_count() && g_wood.w.size() == kWoodAnms * kWoodWords;
    std::vector<uint32_t> cur(kWoodAnms * kWoodWords);
    for (int i = 0; i < kWoodAnms; i++)
        for (int q = 0; q < kWoodWords; q++) cur[i * kWoodWords + q] = ld32(pk + kWoodAnm + kWoodAnmStride * i + 4 * q);
    static int tr = trace_left(7);
    int changed = 0;
    if (valid)
        for (int i = 0; i < kWoodAnms; i++) {
            const uint32_t* a = &g_wood.w[i * kWoodWords];
            const uint32_t* b = &cur[i * kWoodWords];
            if (!memcmp(a, b, 4 * kWoodWords)) continue;
            float d2 = 0;
            for (int r = 0; r < 2; r++)
                for (int q : {3, 7, 11}) d2 += (u32_as_f32(a[12 * r + q]) - u32_as_f32(b[12 * r + q])) * (u32_as_f32(a[12 * r + q]) - u32_as_f32(b[12 * r + q]));
            if (!(d2 < 400.0f * 400.0f)) continue;
            for (int q = 0; q < kWoodWords; q++)
                if (a[q] != b[q]) temp_blend_f32(pk + kWoodAnm + kWoodAnmStride * i + 4 * q, a[q], interp::pass_t());
            if (tr && !changed) {
                tr--;
                LOG("[interp-fx] bush anim %d: sway matrix [0][1] %.4f -> %.4f, halfway %.4f", i, u32_as_f32(a[1]), u32_as_f32(b[1]), u32_as_f32(ld32(pk + kWoodAnm + kWoodAnmStride * i + 4)));
            }
            changed++;
        }
    g_wood.pkt = pk;
    g_wood.step = interp::hold_pass_count();
    g_wood.w = std::move(cur);
}
extern "C" void hook_0254C6C4(Cpu* c) { sway_calc(c, 0, f_0254C6C4_orig); }
extern "C" void hook_025C9948(Cpu* c) { sway_calc(c, 1, f_025C9948_orig); }
extern "C" void hook_02548370(Cpu* c) { sway_calc(c, 2, f_02548370_orig); }

extern "C" void hook_02566B88(Cpu* c) { kankyo_move(c, kRain, f_02566B88_orig); }
extern "C" void hook_02568BD4(Cpu* c) { kankyo_move(c, kSnow, f_02568BD4_orig); }
extern "C" void hook_02569408(Cpu* c) { kankyo_move(c, kSnow, f_02569408_orig); }
extern "C" void hook_02567F68(Cpu* c) { kankyo_move(c, kHousi, f_02567F68_orig); }
extern "C" void hook_0256BB6C(Cpu* c) { kankyo_move(c, kMoya, f_0256BB6C_orig); }
extern "C" void hook_0256CA54(Cpu* c) { kankyo_move(c, kPoison, f_0256CA54_orig); }
extern "C" void f_0257E7C0(Cpu* c);
extern "C" void hook_0256DDF8(Cpu* c) {

    static const int rain = getenv("NSMBU_FORCE_RAIN") ? atoi(getenv("NSMBU_FORCE_RAIN")) : -1;
    if (rain >= 0) {
        uint32_t lr = c->lr, r3 = c->r[3];
        c->r[3] = (uint32_t)rain;
        f_0257E7C0(c);
        c->lr = lr, c->r[3] = r3;
    }
    kankyo_move(c, kVrkumo, f_0256DDF8_orig);
}
extern "C" void hook_0256A388(Cpu* c) { kankyo_move(c, kStar, f_0256A388_orig); }

constexpr uint32_t kClothFly = 0x98, kClothHoist = 0x9C, kClothPos = 0xB0, kClothCur = 0x1C0;
struct ClothRec { uint8_t cur; uint64_t stepped_at; };
std::unordered_map<uint32_t, ClothRec> g_cloth_cur;
extern "C" void hook_0251D864(Cpu* c) {
    uint32_t pk = c->r[3];
    uint8_t cur = (uint8_t)ld8(pk + kClothCur);
    auto it = g_cloth_cur.find(pk);
    const uint64_t step = interp::logic_steps();
    uint64_t stepped_at = it != g_cloth_cur.end() ? it->second.stepped_at : ~0ull;
    const bool changed = it != g_cloth_cur.end() && it->second.cur != cur;
    if (changed) stepped_at = step;

    bool stepped = changed || (interp::hold_pass() && stepped_at == step);
    g_cloth_cur[pk] = ClothRec{cur, stepped_at};
    int32_t fly = (int32_t)ld32(pk + kClothFly), hoist = (int32_t)ld32(pk + kClothHoist);
    if (!on(16) || !halfway() || !stepped || cur > 1 || fly <= 0 || hoist <= 0 || fly * hoist > 4096) {
        f_0251D864_orig(c);
        return;
    }
    const uint32_t n = 3 * (uint32_t)(fly * hoist);
    const float t = interp::pass_t();
    std::vector<std::pair<uint32_t, std::vector<uint32_t>>> saved;
    static int tr = trace_left(6);
    for (uint32_t arr = 0; arr < 3; arr++) {
        uint32_t now = ld32(pk + kClothPos + 8 * arr + 4 * cur), prev = ld32(pk + kClothPos + 8 * arr + 4 * (cur ^ 1));
        if (now < 0x10000000 || now >= 0x50000000 || prev < 0x10000000 || prev >= 0x50000000) continue;
        uint32_t* b = (uint32_t*)ppc_ptr(now);
        const uint32_t* a = (const uint32_t*)ppc_ptr(prev);
        saved.emplace_back(now, std::vector<uint32_t>(b, b + n));
        if (tr && arr == 0) {
            tr--;
            LOG("[interp-fx] cloth %08X vertex 0 x %.3f -> %.3f, drawn %.3f", pk, wf(a[0]), wf(b[0]), lerp_f(wf(a[0]), wf(b[0]), t));
        }
        for (uint32_t q = 0; q < n; q++) b[q] = fw(lerp_f(wf(a[q]), wf(b[q]), t));
    }
    f_0251D864_orig(c);
    for (auto& [addr, w] : saved) memcpy(ppc_ptr(addr), w.data(), 4 * w.size());
}

namespace { void fx_step_stats(); }

namespace interp {
void light_trace_flush();
void fx_pass_start() {
    light_trace_flush();
    g_passes++;
    for (const Applied& a : g_ptcl_applied) write_ptcl(a.ptcl, a.w);
    g_ptcl_keep = std::move(g_ptcl_applied);
    g_ptcl_applied.clear();
    g_wave_keep = 0;
    wave_restore();
    temp_restore();
}

void fx_hold_blend(float t) {
    if (on(1)) ptcl_reapply(t);
    wave_reapply(t);
    temp_reapply(t);
}
}

extern "C" void hook_025AF8A0(Cpu* c) {

    if (!(interp::hold_pass() && !interp::record_pass())) ptcl_restore();
    if (interp::logic_pass()) fx_step_stats();
    f_025AF8A0_orig(c);
}

extern "C" void hook_0282167C(Cpu* c) {
    if (hold_back()) return;
    uint32_t mgr = c->r[3], group = c->r[4];
    if (!on(1) || !halfway() || group >= 16) {
        f_0282167C_orig(c);
        return;
    }
    g_ptcl_before.clear();
    for_each_ptcl(mgr, group, [&](uint32_t p, uint32_t) { g_ptcl_before[p] = read_ptcl(p); });
    static int tr = trace_left(0);
    PtclTrace trace;
    f_0282167C_orig(c);
    static const float kCut = 600.0f;
    static int tr_born = trace_left(5);
    const uint64_t step = interp::hold_pass_count();
    const float t = interp::pass_t();
    for_each_ptcl(mgr, group, [&](uint32_t p, uint32_t emtr) {
        auto it = g_ptcl_before.find(p);
        PtclState cur = read_ptcl(p);
        if (it == g_ptcl_before.end() || cur.age != it->second.age + 1.0f) {
            g_pstats.fresh++;
            if (!on(64)) return;

            auto e = g_emtr.find(emtr);
            if (e == g_emtr.end() || e->second.step != step) return;
            const EmtrRec& er = e->second;
            bool moved = er.prev_step + 1 == step;
            float full[3], d2 = 0;
            for (int i = 0; i < 3; i++) {
                full[i] = wf(*(const uint32_t*)ppc_ptr(p + kPtclVel + 4 * i)) * er.scale[i] + (moved ? er.now[i] - er.prev[i] : 0.0f);
                float d = 0.5f * full[i];
                d2 += d * d;
            }
            if (!(d2 > 1e-8f && d2 < kCut * kCut / 4)) return;
            Applied ap{p};
            memcpy(ap.w, cur.w, sizeof ap.w);
            memcpy(ap.born, full, sizeof full);
            ap.is_born = true;
            uint32_t mid[kPW];
            ptcl_mid(ap, t, mid);
            if (tr_born && d2 > 0.01f) {
                tr_born--;
                LOG("[interp-fx] particle born at %.2f %.2f %.2f, drawn halfway at %.2f %.2f %.2f (emitter moved %.2f %.2f %.2f)",
                    wf(cur.w[0]), wf(cur.w[1]), wf(cur.w[2]), wf(mid[0]), wf(mid[1]), wf(mid[2]), moved ? er.now[0] - er.prev[0] : 0.0f,
                    moved ? er.now[1] - er.prev[1] : 0.0f, moved ? er.now[2] - er.prev[2] : 0.0f);
            }
            g_ptcl_applied.push_back(ap);
            write_ptcl(p, mid);
            g_pstats.born++;
            return;
        }
        const PtclState& a = it->second;
        float d2 = 0;
        for (int i = 0; i < 3; i++) d2 += (wf(a.w[i]) - wf(cur.w[i])) * (wf(a.w[i]) - wf(cur.w[i]));
        if (!(d2 <= kCut * kCut)) return;
        uint32_t mid[kPW];
        mid_ptcl(a.w, cur.w, mid, on(128), t);
        if (!memcmp(mid, cur.w, sizeof mid)) return;
        Applied ap{p};
        memcpy(ap.w, cur.w, sizeof ap.w);
        memcpy(ap.a, a.w, sizeof ap.a);
        ap.is_born = false;
        ap.extras = on(128);
        g_ptcl_applied.push_back(ap);
        write_ptcl(p, mid);
        g_pstats.blended++;
        if (tr) trace.add(a, cur, mid);
    });
    if (tr && (trace.n || trace.an || trace.rn)) {
        trace.log(group);
        tr--;
    }
}

extern "C" void hook_0281FE40(Cpu* c) {
    uint32_t e = c->r[3];
    f_0281FE40_orig(c);
    if (!interp::enabled()) return;
    EmtrRec& r = g_emtr[e];
    const uint64_t step = interp::hold_pass_count();
    if (r.step != step) {
        memcpy(r.prev, r.now, sizeof r.prev);
        r.prev_step = r.step;
    }
    for (int i = 0; i < 3; i++) {
        r.now[i] = (float)ldf32(kEmtrInfo + kInfoCenter + 4 * i);
        r.scale[i] = (float)ldf32(kEmtrInfo + kInfoScale + 4 * i);
    }
    r.step = step;
}

extern "C" void hook_027DF40C(Cpu* c) {
    if (!on(4) || true60::drawing_60()) {
        f_027DF40C_orig(c);
        return;
    }
    uint32_t anm = c->r[3];
    uint32_t ts = ld32(anm);
    if (!ts) {
        f_027DF40C_orig(c);
        return;
    }
    float cur = (float)ldf32(ts);
    if (interp::record_pass()) {
        AnmRec& r = g_anm[anm];
        r.ts = ts;
        r.frame = cur;
        r.pass = interp::hold_pass_count();
        f_027DF40C_orig(c);
        return;
    }
    auto it = g_anm.end();
    if (halfway()) it = g_anm.find(anm);
    if (it == g_anm.end() || it->second.ts != ts || it->second.pass != interp::hold_pass_count() || it->second.frame == cur) {
        f_027DF40C_orig(c);
        return;
    }
    float mid = anm_mid(it->second.frame, cur, (float)ldf32(ts + 4), (float)ldf32(ts + 8), ld32(ts + 0x10) == kLoopFn, interp::pass_t());
    static int tr = trace_left(2);
    if (tr) {
        tr--;
        LOG("[interp-fx] anim %08X frames %.2f -> %.2f, drawn %.2f (%s)", anm, it->second.frame, cur, mid,
            ld32(ts + 0x10) == kLoopFn ? "loop" : "clamp");
    }
    if (mid == cur) {
        f_027DF40C_orig(c);
        return;
    }
    g_astats.blended++;
    stf32(ts, mid);
    f_027DF40C_orig(c);
    stf32(ts, cur);
}

namespace interp {
bool fx_hold_anim() { return hold_back() && !in_execute(); }
}

namespace {
uint32_t g_plight_tevstr = 0;
struct RndRec {
    std::unordered_map<uint64_t, std::vector<double>> vals;
    std::unordered_map<uint64_t, uint32_t> next;
    uint64_t frame = ~0ull;
};
RndRec g_rnd_logic, g_rnd_hold;
uint64_t rnd_key(Cpu* c, char fn) {
    if (g_plight_tevstr) return (1ull << 63) | ((uint64_t)fn << 32) | g_plight_tevstr;
    return ((uint64_t)fn << 32) | c->lr;
}
}

namespace {
const bool g_light_trace = getenv("NSMBU_LIGHT_TRACE") != nullptr;
std::string g_light_line;
}
namespace interp {
void light_trace_flush() {
    if (!g_light_trace || g_light_line.empty()) return;
    LOG("[light] step %llu %s%s", (unsigned long long)interp::logic_steps(), interp::hold_pass() ? "H" : "L", g_light_line.c_str());
    g_light_line.clear();
}
void light_trace_add(const char* fmt, ...) {
    if (!g_light_trace) return;
    char b[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(b, sizeof b, fmt, ap);
    va_end(ap);
    g_light_line += b;
}
}
extern "C" void f_025615B8_orig(Cpu* c);

namespace {
struct PlightPos {
    uint64_t step = ~0ull, restored = ~0ull;
    uint32_t w[3];
};
std::unordered_map<uint32_t, PlightPos> g_plight_pos;
void plight_pos_hold(uint32_t ts) {
    if (!on(8) || interp::in_execute()) return;
    const uint64_t step = interp::logic_steps();
    if (!interp::hold_pass()) {
        if (g_plight_pos.size() > 4096) g_plight_pos.clear();
        PlightPos& p = g_plight_pos[ts];
        if (p.step == step) return;
        p.step = step;
        for (int i = 0; i < 3; i++) p.w[i] = ld32(ts + 0x84 + 4 * i);
        return;
    }
    auto it = g_plight_pos.find(ts);
    if (it == g_plight_pos.end() || it->second.step != step || it->second.restored == interp::pass_count()) return;
    it->second.restored = interp::pass_count();
    for (int i = 0; i < 3; i++) st32(ts + 0x84 + 4 * i, it->second.w[i]);
}
}
extern "C" void hook_025615B8(Cpu* c) {
    uint32_t saved = g_plight_tevstr;
    uint32_t ts = c->r[5];
    g_plight_tevstr = c->r[5];
    plight_pos_hold(ts);
    f_025615B8_orig(c);
    g_plight_tevstr = saved;
    if (g_light_trace)
        interp::light_trace_add(" | %X r%u w(%.1f,%.1f,%.1f) v(%.1f,%.1f,%.1f)", ts & 0xFFFFFF, ld8(ts + 0x18), ldf32(ts + 0x84), ldf32(ts + 0x88),
                                ldf32(ts + 0x8C), ldf32(ts + 0), ldf32(ts + 4), ldf32(ts + 8));
}

void rnd_trace(Cpu* c, char fn, bool replayed) {
    static const bool on_ = getenv("NSMBU_RND_TRACE") != nullptr;
    if (!on_ || !interp::fx_hold_anim()) return;
    static std::unordered_map<uint64_t, int> n;
    static int total = 0, hits = 0;
    n[((uint64_t)fn << 32) | c->lr]++;
    hits += replayed;
    if (++total % 300) return;
    std::vector<std::pair<int, uint64_t>> v;
    for (auto& e : n) v.push_back({e.second, e.first});
    std::sort(v.rbegin(), v.rend());
    LOG("[rnd] last 300 calls on frames without logic: %d replayed", hits);
    for (size_t i = 0; i < v.size() && i < 12; i++)
        LOG("[rnd]   caller %08X (cM_rnd%c): %d", (uint32_t)v[i].second, (char)(v[i].second >> 32), v[i].first);
    n.clear();
    hits = 0;
}

static void rnd_call(Cpu* c, char fn, void (*orig)(Cpu*)) {
    static int depth = 0;

    static FILE* lf = getenv("NSMBU_RND_LOG") ? fopen(getenv("NSMBU_RND_LOG"), "w") : nullptr;
    static const uint32_t bt_lo = getenv("NSMBU_RND_BT") ? (uint32_t)strtoul(getenv("NSMBU_RND_BT"), nullptr, 16) : 0;
    if (lf && bt_lo && (c->lr >> 12) == (bt_lo >> 12)) {
        uint32_t sp = c->r[1];
        fprintf(lf, "BT");
        for (int k = 0; k < 10 && sp >= 0x10000000; k++) { sp = ld32(sp); if (sp < 0x10000000) break; fprintf(lf, " %08X", ld32(sp + 4)); }
        fprintf(lf, "\n");
    }
    if (lf && !depth && (c->lr == GC(0x025616CCu) || getenv("NSMBU_RND_REGS_ALL")))
        fprintf(lf, "REGS %08X %08X %08X %08X %08X %08X %08X L %08X %08X %08X %08X %08X\n", c->r[24], c->r[26], c->r[27], c->r[28], c->r[29], c->r[30], c->r[31],
                ld32(c->r[24] + 0x14), ld32(c->r[24] + 0x18), ld32(c->r[24] + 0x1C), ld32(c->r[24] + 0x28), ld32(c->r[24] + 0x2C));
    if (lf && !depth) fprintf(lf, "%llu %d %d %c %08X %08X\n", (unsigned long long)interp::logic_steps(), (int)interp::hold_pass(),
                               (int)interp::in_execute(), fn, c->lr, ld32(GD(0x101FF9D4)));
    if (!on(8) || interp::in_execute() || depth) {
        orig(c);
        return;
    }
    struct Nest { Nest() { depth++; } ~Nest() { depth--; } } nest;
    double max = fn == ' ' ? 1.0 : c->f[1].ps0;
    auto to_frac = [&](double v) { return max == 0 ? 0.0 : fn == 'X' ? (v / max + 1.0) * 0.5 : v / max; };
    auto from_frac = [&](double f) { return fn == 'X' ? (f - 0.5) * 2.0 * max : f * max; };
    uint64_t key = rnd_key(c, fn);
    if (interp::fx_hold_anim()) {
        RndRec& h = g_rnd_hold;
        if (h.frame != interp::pass_count()) h.frame = interp::pass_count(), h.next.clear();
        uint32_t i = h.next[key]++;
        auto it = g_rnd_logic.vals.find(key);
        bool hit = it != g_rnd_logic.vals.end() && i < it->second.size();
        rnd_trace(c, fn, hit);
        if (hit) {
            c->f[1].ps0 = (float)from_frac(it->second[i]);
            return;
        }

        uint32_t seeds[3];
        bool keep = true60::enabled();
        if (keep)
            for (int k = 0; k < 3; k++) seeds[k] = ld32(GD(0x101FF9D4) + 4 * k);
        orig(c);
        if (keep)
            for (int k = 0; k < 3; k++) st32(GD(0x101FF9D4) + 4 * k, seeds[k]);
        return;
    }
    RndRec& r = g_rnd_logic;
    if (r.frame != interp::logic_steps()) r.frame = interp::logic_steps(), r.vals.clear();
    orig(c);
    auto& v = r.vals[key];
    if (v.size() < 64) v.push_back(to_frac(c->f[1].ps0));
}
extern "C" void f_02019788_orig(Cpu* c);
extern "C" void f_020198D8_orig(Cpu* c);
extern "C" void f_02019918_orig(Cpu* c);
extern "C" void hook_02019788(Cpu* c) { rnd_call(c, ' ', f_02019788_orig); }
extern "C" void hook_020198D8(Cpu* c) { rnd_call(c, 'F', f_020198D8_orig); }
extern "C" void hook_02019918(Cpu* c) { rnd_call(c, 'X', f_02019918_orig); }

extern "C" void hook_0246C5E8(Cpu* c) {
    uint32_t pk = c->r[3];
    uint32_t tab = ld32(pk + kSeaHeights);
    if (!on(2) || !tab) {
        f_0246C5E8_orig(c);
        return;
    }
    uint32_t* h = (uint32_t*)ppc_ptr(tab);
    float min_x = (float)ldf32(pk + kSeaMinX), min_z = (float)ldf32(pk + kSeaMinZ);
    if (interp::record_pass()) {
        g_sea.packet = pk;
        g_sea.pass = interp::hold_pass_count();
        g_sea.min_x = min_x;
        g_sea.min_z = min_z;
        g_sea.h.assign(h, h + kSeaCells);
        f_0246C5E8_orig(c);
        return;
    }
    if (!halfway() || g_sea.packet != pk || g_sea.pass != interp::hold_pass_count() || g_sea.h.size() != kSeaCells ||
        !(std::fabs(min_x - g_sea.min_x) < 2000.0f && std::fabs(min_z - g_sea.min_z) < 2000.0f)) {
        f_0246C5E8_orig(c);
        return;
    }
    const float t = interp::pass_t();
    std::vector<uint32_t> exact(h, h + kSeaCells);
    for (int i = 0; i < kSeaCells; i++) h[i] = fw(lerp_f(wf(g_sea.h[i]), wf(exact[i]), t));
    stf32(pk + kSeaMinX, t == 0.5f ? 0.5f * (min_x + g_sea.min_x) : lerp_f(g_sea.min_x, min_x, t));
    stf32(pk + kSeaMinZ, t == 0.5f ? 0.5f * (min_z + g_sea.min_z) : lerp_f(g_sea.min_z, min_z, t));
    static int tr = trace_left(1);
    if (tr) {
        tr--;
        int k = 0;
        for (int i = 1; i < kSeaCells; i++)
            if (std::fabs(wf(exact[i]) - wf(g_sea.h[i])) > std::fabs(wf(exact[k]) - wf(g_sea.h[k]))) k = i;
        LOG("[interp-fx] sea vertex %d,%d height %.3f -> %.3f, drawn %.3f; origin x %.1f -> %.1f", k % 65, k / 65, wf(g_sea.h[k]), wf(exact[k]),
            wf(h[k]), g_sea.min_x, min_x);
    }
    f_0246C5E8_orig(c);
    memcpy(h, exact.data(), 4 * kSeaCells);
    stf32(pk + kSeaMinX, min_x);
    stf32(pk + kSeaMinZ, min_z);
}

extern "C" void hook_0246C7C8(Cpu* c) {
    uint32_t pk = c->r[3];
    if (hold_back() && on(2)) {
        int16_t v = (int16_t)ld16(pk + kSeaScroll);
        st16(pk + kSeaScroll, (uint16_t)(v <= 0 ? 300 : v - 1));
    }
    g_sea_scroll_half = on(2) && halfway();
    f_0246C7C8_orig(c);
    g_sea_scroll_half = false;
}

extern "C" void hook_0246BD4C(Cpu* c) {
    static const bool force = getenv("NSMBU_SEA_WAVES") != nullptr;
    uint32_t pk = c->r[3];
    f_0246BD4C_orig(c);
    if (force) {
        stf32(pk + 0x130, 1.0f);
        stf32(pk + 0xF0 + 0x24, 1.0f);
    }
}

extern "C" void hook_028E93CC(Cpu* c) {
    if (g_sea_scroll_half && c->lr == GC(0x0246C958)) {
        static int tr = trace_left(3);
        double y = c->f[2].ps0 - (1.0 - (double)interp::pass_t()) / 300.0;
        if (tr) {
            tr--;
            LOG("[interp-fx] sea scroll %.5f, drawn %.5f", c->f[2].ps0, y);
        }
        c->f[2].ps0 = y;
    }
    f_028E93CC_orig(c);
}

namespace {

void fx_step_stats() {
    static uint64_t n = 0;
    if (++n % 300) return;
    static const bool stats = getenv("NSMBU_INTERP_FX_STATS") != nullptr;
    if (stats)
        LOG("[interp-fx] per step: %.1f particles blended, %.1f new (%.1f drawn half a step back); anims %.1f blended, %.2f wrapped, %.2f jumped",
            g_pstats.blended / 300.0, g_pstats.fresh / 300.0, g_pstats.born / 300.0, g_astats.blended / 300.0, g_astats.wrapped / 300.0, g_astats.jumped / 300.0);
    g_pstats = {};
    g_astats = {};
    uint64_t now = interp::hold_pass_count();
    for (auto it = g_anm.begin(); it != g_anm.end();) it = it->second.pass + 2 < now ? g_anm.erase(it) : std::next(it);
    for (auto it = g_emtr.begin(); it != g_emtr.end();) it = it->second.step + 2 < now ? g_emtr.erase(it) : std::next(it);
}
}

namespace {
constexpr uint32_t kMorfFrameCtrl = 0x98;
struct AttRec {
    float pos[3], frame;
    uint64_t pass;
};
std::unordered_map<uint32_t, AttRec> g_att;
}
extern "C" void hook_024EC1C8(Cpu* c) {
    uint32_t self = c->r[3], pos = c->r[4];
    uint32_t morf = ld32(self);
    if (!true60::enabled() || !interp::enabled() || !on(16) || morf < 0x10000000 || morf >= 0x50000000) {
        f_024EC1C8_orig(c);
        return;
    }
    const uint32_t fc = morf + kMorfFrameCtrl;
    if (interp::record_pass()) {
        AttRec& r = g_att[self];
        for (int i = 0; i < 3; i++) r.pos[i] = (float)ldf32(pos + 4 * i);
        r.frame = (float)ldf32(fc + 4);
        r.pass = interp::hold_pass_count();
        f_024EC1C8_orig(c);
        return;
    }
    if (!halfway()) {
        f_024EC1C8_orig(c);
        return;
    }
    const uint32_t mid_pos = mem::fixed_slot(mem::kFixFxMidPos);
    auto it = g_att.find(self);
    bool valid = it != g_att.end() && it->second.pass == interp::hold_pass_count();
    float d2 = 0, mp[3];
    for (int i = 0; i < 3; i++) {
        float cur = (float)ldf32(pos + 4 * i);
        mp[i] = valid ? lerp_f(it->second.pos[i], cur, interp::pass_t()) : cur;
        d2 += (mp[i] - cur) * (mp[i] - cur);
    }
    if (!(d2 < 200.0f * 200.0f)) for (int i = 0; i < 3; i++) mp[i] = (float)ldf32(pos + 4 * i);
    for (int i = 0; i < 3; i++) stf32(mid_pos + 4 * i, mp[i]);
    float frame = (float)ldf32(fc + 4);
    float mf = valid ? anm_mid(it->second.frame, frame, (float)(int16_t)ld16(fc + 8), (float)(int16_t)ld16(fc + 0xA), true, interp::pass_t()) : frame;
    static int tr = trace_left(6);
    if (tr) {
        tr--;
        LOG("[interp-fx] attention arrow (true 60, full pass): target %.2f %.2f %.2f -> drawn at %.2f %.2f %.2f, frame %.2f -> %.2f",
            ldf32(pos), ldf32(pos + 4), ldf32(pos + 8), mp[0], mp[1], mp[2], frame, mf);
    }
    stf32(fc + 4, mf);
    c->r[4] = mid_pos;
    true60::force_draw_60(true);
    f_024EC1C8_orig(c);
    true60::force_draw_60(false);
    stf32(fc + 4, frame);
}

namespace interp {
void fx_ss_reset() {
    g_ptcl_before.clear();
    g_ptcl_applied.clear();
    g_ptcl_keep.clear();
    g_emtr.clear();
    g_anm.clear();
    g_temp.clear();
    g_temp_keep.clear();
    g_wave.pkt = 0;
    g_wave_keep = 0;
    g_wave_pkt = 0;
    g_cloth_cur.clear();
    g_att.clear();
    for (auto& s : g_sway) s = SwayRec{};
    g_wood = WoodRec{};
    g_sea.packet = 0;
    g_sea.h.clear();
    g_sea_scroll_half = false;
}
}
