

#pragma once
#include <cstdint>
#include <string>

namespace render {

enum class Api : int { Metal = 0, Vulkan = 1 };
const char* api_name(Api a);
const char* api_key(Api a);
bool compiled(Api a);
bool can_choose();

struct Backend {
    Api api;

    void (*init)();
    void (*run_main_loop)();

    void (*draw)(const uint32_t* regs, uint32_t prim, uint32_t count, uint32_t indexType, uint32_t indexAddr,
                 uint32_t baseVertex, uint32_t instances);
    void (*clear_color)(const uint32_t* regs, uint32_t colorBuffer, const float rgba[4]);
    void (*clear_depth_stencil)(const uint32_t* regs, uint32_t depthBuffer, float depth, uint32_t stencil, uint32_t flags);
    void (*copy_surface)(uint32_t src, uint32_t srcMip, uint32_t srcSlice, uint32_t dst, uint32_t dstMip, uint32_t dstSlice);
    void (*copy_to_scan)(uint32_t colorBuffer, uint32_t target);
    void (*swap)();
    void (*peek_z)(const uint32_t* cells, uint32_t words);
    void (*set_frame_aspect)(float a);
    bool (*target_aspect_factors)(uint32_t w, uint32_t h, float& kx, float& ky);
    uint64_t (*frames_completed)();
    void (*with_autorelease_pool)(void (*fn)());
    void (*set_tv_format)(uint32_t gx2Format, bool tv);
    void (*invalidate)(uint32_t flags, uint32_t addr, uint32_t size);
    void (*guest_flush)();
    void (*wait_idle)();
    void (*write_back)();
    void (*ss_reset)();

    uint64_t (*frame_count)();
    void (*request_tv_dump)(const std::string& path, int frames_ahead);
    void (*request_capture)();
    void (*shutdown)();

    float (*res_scale)();
    void (*set_res_scale)(float);
    int (*ao_mode)();
    void (*set_ao_mode)(int);
    bool (*ao_hires)();
    void (*set_ao_hires)(bool);
    bool (*aniso)();
    void (*set_aniso)(bool);
    bool (*fxaa)();
    void (*set_fxaa)(bool);

    bool (*feature_available)(int feature);

    std::string (*device)();
};
enum Feature : int { kFeatureAO, kFeatureAOHires, kFeatureAniso, kFeatureFXAA, kFeatureScaleFilter, kFeatureCapture,
                     kFeatureShaderHeadStart };

extern const Backend* g_backend;
#ifdef NSMBU_HAS_METAL
const Backend& metal_backend();
#endif
#ifdef NSMBU_HAS_VULKAN
const Backend& vulkan_backend();
#endif

void choose(int argc, char** argv);

void set_restart_args(int argc, char** argv);

void init();
void run_main_loop();

Api active();
inline bool vulkan() { return g_backend && g_backend->api == Api::Vulkan; }
Api requested();
std::string fallback_reason();
Api preferred();
void set_preferred(Api a);
bool restart_pending();

bool restart();

inline void draw(const uint32_t* regs, uint32_t prim, uint32_t count, uint32_t indexType, uint32_t indexAddr,
                 uint32_t baseVertex, uint32_t instances) {
    g_backend->draw(regs, prim, count, indexType, indexAddr, baseVertex, instances);
}
inline void clear_color(const uint32_t* regs, uint32_t cb, const float rgba[4]) { g_backend->clear_color(regs, cb, rgba); }
inline void clear_depth_stencil(const uint32_t* regs, uint32_t db, float d, uint32_t s, uint32_t f) {
    g_backend->clear_depth_stencil(regs, db, d, s, f);
}
inline void copy_surface(uint32_t src, uint32_t srcMip, uint32_t srcSlice, uint32_t dst, uint32_t dstMip, uint32_t dstSlice) {
    g_backend->copy_surface(src, srcMip, srcSlice, dst, dstMip, dstSlice);
}
inline void copy_to_scan(uint32_t cb, uint32_t target) { g_backend->copy_to_scan(cb, target); }
inline void swap() { g_backend->swap(); }
inline void peek_z(const uint32_t* cells, uint32_t words) { g_backend->peek_z(cells, words); }
inline void set_frame_aspect(float a) { g_backend->set_frame_aspect(a); }
inline bool target_aspect_factors(uint32_t w, uint32_t h, float& kx, float& ky) { return g_backend->target_aspect_factors(w, h, kx, ky); }
inline uint64_t frames_completed() { return g_backend->frames_completed(); }
inline void with_autorelease_pool(void (*fn)()) { g_backend->with_autorelease_pool(fn); }
inline void set_tv_format(uint32_t f, bool tv) { g_backend->set_tv_format(f, tv); }
inline void invalidate(uint32_t flags, uint32_t addr, uint32_t size) { g_backend->invalidate(flags, addr, size); }
inline void guest_flush() { g_backend->guest_flush(); }
inline void wait_idle() { g_backend->wait_idle(); }
inline void write_back() { if (g_backend->write_back) g_backend->write_back(); }
inline void ss_reset() { g_backend->ss_reset(); }
uint64_t frame_count();
inline void request_tv_dump(const std::string& path, int frames_ahead) { g_backend->request_tv_dump(path, frames_ahead); }
inline void request_capture() { g_backend->request_capture(); }
void shutdown();

inline float res_scale() { return g_backend->res_scale(); }
inline void set_res_scale(float f) { g_backend->set_res_scale(f); }
inline int ao_mode() { return g_backend->ao_mode(); }
inline void set_ao_mode(int m) { g_backend->set_ao_mode(m); }
inline bool ao_hires() { return g_backend->ao_hires(); }
inline void set_ao_hires(bool v) { g_backend->set_ao_hires(v); }
inline bool aniso() { return g_backend->aniso(); }
inline void set_aniso(bool v) { g_backend->set_aniso(v); }

float bloom_strength();
void set_bloom_strength(float strength);

void scale_bloom_uniforms(void* remapped, size_t size);
inline bool fxaa() { return g_backend->fxaa(); }
inline void set_fxaa(bool v) { g_backend->set_fxaa(v); }
inline bool feature_available(Feature f) { return g_backend->feature_available(f); }
inline std::string device() { return g_backend && g_backend->device ? g_backend->device() : std::string(); }

}
