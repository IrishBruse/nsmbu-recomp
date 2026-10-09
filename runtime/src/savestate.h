

#pragma once
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

#include "ppc.h"

namespace ss {

struct Writer {
    std::vector<uint8_t> b;
    void bytes(const void* p, size_t n) { b.insert(b.end(), (const uint8_t*)p, (const uint8_t*)p + n); }
    template <class T> void pod(const T& v) {
        static_assert(std::is_trivially_copyable<T>::value, "pod");
        bytes(&v, sizeof v);
    }
    void u8(uint8_t v) { pod(v); }
    void u32(uint32_t v) { pod(v); }
    void u64(uint64_t v) { pod(v); }
    void str(const std::string& s) { u32((uint32_t)s.size()); bytes(s.data(), s.size()); }
};

struct Reader {
    const uint8_t* p = nullptr;
    const uint8_t* e = nullptr;
    bool ok = true;
    Reader() = default;
    Reader(const void* d, size_t n) : p((const uint8_t*)d), e((const uint8_t*)d + n) {}
    bool bytes(void* out, size_t n) {
        if (!ok || (size_t)(e - p) < n) { ok = false; if (out) memset(out, 0, n); return false; }
        if (out) memcpy(out, p, n);
        p += n;
        return true;
    }
    template <class T> T pod() {
        T v{};
        bytes(&v, sizeof v);
        return v;
    }
    uint8_t u8() { return pod<uint8_t>(); }
    uint32_t u32() { return pod<uint32_t>(); }
    uint64_t u64() { return pod<uint64_t>(); }
    std::string str() {
        uint32_t n = u32();
        if (!ok || (size_t)(e - p) < n) { ok = false; return {}; }
        std::string s((const char*)p, n);
        p += n;
        return s;
    }
    bool at_end() const { return p == e; }
};

constexpr int kSlots = 5;
struct SlotInfo {
    bool used = false;
    bool compatible = true;
    bool portable = false;
    std::string when;
    std::string controller;
    std::string area;
    std::string path;
    bool older_other = false;
    uint64_t older_bytes = 0;
};
SlotInfo slot_info(int slot);
void request_save(int slot);

void request_save(int slot, std::function<void(bool ok, const std::string& why)> done);
void request_save_portable(int slot);
void request_save_full(int slot);
bool in_gameplay();
void request_load(int slot);
void request_load_portable_file(const std::string& path);
bool full_states();
void set_full_states(bool on);
bool full_states_forced();
std::string bug_report_text();
std::string states_dir();
std::string last_message();

void notice(const std::string& text);

void service(Cpu* c);

uint32_t snap_ld32(uint32_t ea);

uint64_t last_load_frame();
uint64_t last_load_step();
uint32_t last_load_counter();
}
