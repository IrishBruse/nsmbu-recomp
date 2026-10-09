#pragma once
#include "ppc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NSMBU_GUEST_ABI_VERSION 2

enum { NSMBU_GUEST_REPLACE = 1, NSMBU_GUEST_HOOK_ENTRY = 2, NSMBU_GUEST_HOOK_RETURN = 3 };

typedef struct NSMBUGuestHostV1 {
    uint32_t size, abi_version;
    void (*dispatch)(Cpu* c);
    void (*unimplemented)(Cpu* c, uint32_t addr, uint32_t insn);
    void (*trap)(Cpu* c, uint32_t addr);
    uint64_t (*timebase)(void);
    double (*fres)(double);
    double (*frsqrte)(double);
    void (*preempt)(Cpu* c);
    volatile int* core_preempt;
    void (*call_original)(Cpu* c, uint32_t func);
    PpcFunc (*service)(const char* name);
} NSMBUGuestHostV1;

typedef struct { uint32_t addr; PpcFunc fn; } NSMBUGuestFunc;
typedef struct { uint32_t kind, target, func, flags; PpcFunc fn; } NSMBUGuestHook;

typedef struct NSMBUGuestModuleV1 {
    uint32_t size, abi_version;
    const char* translator;
    uint32_t mem_base, mem_size;
    const uint8_t* image;
    uint32_t image_size;
    const NSMBUGuestFunc* funcs;
    uint32_t func_count;
    const NSMBUGuestHook* hooks;
    uint32_t hook_count;
} NSMBUGuestModuleV1;

typedef const NSMBUGuestModuleV1* (*NSMBUGuestInitV1)(const NSMBUGuestHostV1* host);
#define NSMBU_GUEST_INIT_SYMBOL "nsmbu_guest_module_v1"

#ifdef __cplusplus
}
#endif
