#include "../runtime.h"

#include <mutex>
#include <unordered_map>
#include <zlib.h>

namespace {

struct Stream {
    z_stream z{};
    bool init = false;
};

std::mutex g_mu;
std::unordered_map<uint32_t, Stream> g_streams;

void pull(Stream& s, uint32_t g) {
    s.z.next_in = (Bytef*)mem::ptr(ld32(g + 0));
    s.z.avail_in = ld32(g + 4);
    s.z.next_out = (Bytef*)mem::ptr(ld32(g + 12));
    s.z.avail_out = ld32(g + 16);
}

void push(Stream& s, uint32_t g, uint32_t in_ptr, uint32_t in_avail, uint32_t out_ptr, uint32_t out_avail) {
    uint32_t in_used = in_avail - s.z.avail_in;
    uint32_t out_used = out_avail - s.z.avail_out;
    st32(g + 0, in_ptr + in_used);
    st32(g + 4, s.z.avail_in);
    st32(g + 8, ld32(g + 8) + in_used);
    st32(g + 12, out_ptr + out_used);
    st32(g + 16, s.z.avail_out);
    st32(g + 20, ld32(g + 20) + out_used);
    st32(g + 44, (uint32_t)s.z.data_type);
    st32(g + 48, (uint32_t)s.z.adler);
}

}  // namespace

HLE(zlib125, inflateInit2_) {
    uint32_t g = arg(c, 0);
    std::lock_guard<std::mutex> lk(g_mu);
    Stream& s = g_streams[g];
    if (s.init) {
        inflateEnd(&s.z);
        s = Stream{};
    }
    int rc = inflateInit2(&s.z, (int)arg(c, 1));
    s.init = rc == Z_OK;
    if (s.init) st32(g + 28, 1);
    ret(c, (uint32_t)rc);
}

HLE(zlib125, inflate) {
    uint32_t g = arg(c, 0);
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_streams.find(g);
    if (it == g_streams.end() || !it->second.init) {
        ret(c, (uint32_t)Z_STREAM_ERROR);
        return;
    }
    uint32_t in_ptr = ld32(g + 0), out_ptr = ld32(g + 12);
    uint32_t in_avail = ld32(g + 4), out_avail = ld32(g + 16);
    pull(it->second, g);
    int rc = inflate(&it->second.z, (int)arg(c, 1));
    push(it->second, g, in_ptr, in_avail, out_ptr, out_avail);
    ret(c, (uint32_t)rc);
}

HLE(zlib125, inflateEnd) {
    uint32_t g = arg(c, 0);
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_streams.find(g);
    int rc = Z_STREAM_ERROR;
    if (it != g_streams.end() && it->second.init) {
        rc = inflateEnd(&it->second.z);
        g_streams.erase(it);
        st32(g + 28, 0);
    }
    ret(c, (uint32_t)rc);
}
