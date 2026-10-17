
#include "gfx/depth_peek.h"

#include <atomic>
#include <mutex>
#include <unordered_map>

#include "runtime.h"

namespace gfx::depth_peek {
uint64_t next_ticket() { static std::atomic<uint64_t> serial{0}; return ++serial; }
void publish(uint64_t ticket, const std::vector<uint32_t>& destinations, const std::vector<uint32_t>& depths) {
    static std::mutex mutex;
    static std::unordered_map<uint32_t, uint64_t> completed;
    std::lock_guard lock(mutex);
    for (size_t i = 0; i < destinations.size(); i++) {
        uint32_t dst = destinations[i];
        if (!dst || completed[dst] >= ticket) continue;
        completed[dst] = ticket;
        st32(dst, depths[i]);
    }
}
}
