

#include <algorithm>
#include <cstdlib>
#include <vector>

#include "gx2/gx2_cmd.h"
#include "runtime.h"

extern "C" void f_0252E388_orig(Cpu* c);

extern "C" void hook_0252E388(Cpu* c) {
    static const bool off = [] { const char* e = getenv("NSMBU_PEEKZ"); return e && atoi(e) == 0; }();
    if (off) { f_0252E388_orig(c); return; }
    const uint32_t obj = c->r[3];
    const uint32_t n = std::min<uint32_t>(ld8(obj), 64);
    std::vector<uint32_t> cells;
    cells.reserve(n * 3);
    for (uint32_t i = 0; i < n; i++) {
        uint32_t cell = obj + 4 + i * 8;
        uint32_t dst = ld32(cell + 4);
        if (!dst) continue;
        cells.push_back((uint32_t)(int32_t)(int16_t)ld16(cell));
        cells.push_back((uint32_t)(int32_t)(int16_t)ld16(cell + 2));
        cells.push_back(dst);
    }
    if (!cells.empty()) gx2::emit(gx2::OP_PEEK_Z, cells.data(), (uint32_t)cells.size());
    st8(obj, 0);
}

#include "gfx/depth_peek.h"
#include <atomic>
#include <mutex>
#include <unordered_map>
namespace gfx::depth_peek {
uint64_t next_ticket() { static std::atomic<uint64_t> serial{0}; return ++serial; }
void publish(uint64_t ticket, const std::vector<uint32_t>& destinations, const std::vector<uint32_t>& depths) {

    static std::mutex mutex;
    static std::unordered_map<uint32_t, uint64_t> completed;
    std::lock_guard lock(mutex);
    static const bool log = getenv("NSMBU_PEEKZ_LOG") != nullptr;
    for (size_t i = 0; i < destinations.size(); i++) {
        uint32_t dst = destinations[i];
        if (!dst || completed[dst] >= ticket) continue;
        completed[dst] = ticket;
        st32(dst, depths[i]);
        if (log) LOG("[peekz] %08X: %06X", dst, depths[i]);
    }
}
}
