

#pragma once
#include <cstdint>
#include <string>

namespace rprof {

enum Phase : int {

    kShader, kIndices, kTargets, kPipeline, kUniforms, kTextures, kDescriptors, kPass, kRecord, kVertex,
    kSubmit, kDrawPhases
};
enum Op : int { kOpRegs, kOpDraw, kOpClear, kOpCopy, kOpScan, kOpInvalidate, kOpFlush, kOpDrawDone, kOpSwap, kOpOther, kOps };
enum Upload : int { kUpOther, kUpVertex, kUpIndex, kUpUbo, kUpUniformVars, kUpTexture, kUploadKinds };
enum Wait : int { kWaitGpu, kWaitAcquire, kWaitPresent, kWaits };
enum Sync : int { kSyncDrawDone, kSyncCopySurface, kSyncFlip, kSyncOther, kSyncs };

bool enabled();
uint64_t now_ns();
uint64_t thread_cpu_ns();

extern bool g_draw_sampled;
extern uint64_t g_mark;
void mark_slow(Phase p);
inline void mark(Phase p) {
    if (g_draw_sampled) mark_slow(p);
}

uint64_t op_begin(Op op);
void op_end(Op op, uint64_t started);

extern int g_upload_kind;
struct UploadKind {
    int saved;
    explicit UploadKind(Upload k) : saved(g_upload_kind) { g_upload_kind = k; }
    ~UploadKind() { g_upload_kind = saved; }
};
void add_upload(uint64_t bytes);
void add_upload(Upload kind, uint64_t bytes);

extern bool g_track_unique;
void guest_read_slow(Upload kind, uint32_t addr, uint64_t size);
inline void guest_read(Upload kind, uint32_t addr, uint64_t size) {
    if (g_track_unique) guest_read_slow(kind, addr, size);
}

void add_wait(Wait w, uint64_t ns);
void add_idle(uint64_t ns);

extern uint32_t g_reg_dirty;
void note_other_reg(uint32_t reg);
void classify_draw();
bool fast_class_reg(uint32_t reg);

void shader_variant(bool newProgram, bool onlyUnusedUnits, const char* const* groups, int groupCount);

void frame_end(bool hold);

void add_sync(Sync site, uint64_t ns);

void add_write_back(uint64_t walkNs, uint32_t surfaces, uint64_t bytes, uint64_t readNs);

std::string latest_report();

}
