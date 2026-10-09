

#pragma once
#include <cstddef>
#include <cstdint>

namespace wwatch {

bool init(uint8_t* base, uint64_t size);
bool active();

uint64_t arm(uint32_t addr, uint32_t size);
bool written_since(uint32_t addr, uint32_t size, uint64_t stamp);

bool changed_since(uint32_t addr, uint32_t size, uint64_t stamp);
uint64_t write_seq();

void enable_hints();
void hint(uint32_t addr, uint32_t size);

void host_write_begin(uint32_t addr, uint32_t size);
void host_write_end(uint32_t addr, uint32_t size);
struct HostWrite {
    uint32_t addr, size;
    HostWrite(uint32_t a, uint32_t n) : addr(a), size(n) { host_write_begin(a, n); }
    ~HostWrite() { host_write_end(addr, size); }
};

void take_stats(uint64_t& faults, uint64_t& protectedPages);

void take_hint_stats(uint64_t& calls, uint64_t& bytes);

uint64_t protect_failures();

}
