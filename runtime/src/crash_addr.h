

#pragma once
#include <cstddef>
#include <cstdint>

namespace crash_addr {

using Out = void (*)(int fd, const char* s, size_t n);

inline int fit(int n, size_t cap) { return n < 0 ? 0 : (size_t)n >= cap ? (int)cap - 1 : n; }

int describe(char* buf, size_t cap, uintptr_t addr, char* path = nullptr, size_t path_cap = 0);

void host_backtrace(int fd, Out out, const void* context);

#ifndef _WIN32

uintptr_t context_pc(const void* ucontext);
#endif

void prime();

}
