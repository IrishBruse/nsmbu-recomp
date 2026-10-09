
#pragma once
#include <stdint.h>

#include "ppc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CalleeInfo {
    uint32_t addr;
    const char* name;
    uint32_t imask;
    uint32_t fmask;
    uint32_t defr;
    uint32_t deff;
    uint8_t ptrsz[11];
    uint8_t outp[11];
    void (*real)(Cpu*);
    uint8_t declared;
    uint8_t retkind;
    uint8_t retbits;
    uint32_t retconst;
    uint8_t nstack;
} CalleeInfo;

typedef struct OrigFunc {
    uint32_t addr;
    const char* name;
    const char* gcsym;
    void (*fn)(Cpu*);
    uint32_t nblocks;
    char argtype[11];
    uint8_t nflt;
    uint32_t livein_r, livein_f;
} OrigFunc;

extern const CalleeInfo unit_callees[];
extern const unsigned unit_ncallees;
extern const OrigFunc unit_funcs[];
extern const unsigned unit_nfuncs;
extern const char unit_name[];

#ifdef __cplusplus
}
#endif
