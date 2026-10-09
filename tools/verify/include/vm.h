

#pragma once
#include <stdint.h>

#include "ppc.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t vm_ld8(uint32_t ea);
uint16_t vm_ld16(uint32_t ea);
uint32_t vm_ld32(uint32_t ea);
uint64_t vm_ld64(uint32_t ea);
void vm_st8(uint32_t ea, uint8_t v);
void vm_st16(uint32_t ea, uint16_t v);
void vm_st32(uint32_t ea, uint32_t v);
void vm_st64(uint32_t ea, uint64_t v);

void vm_call(Cpu* c, uint32_t target, int kind);
enum { VM_CALL_DIRECT = 0, VM_CALL_INDIRECT = 1, VM_CALL_IMPORT = 2 };

void vm_cov(uint32_t func, uint32_t block);

void vm_trap(Cpu* c, uint32_t addr, uint32_t insn);

#ifdef __cplusplus
}
#endif
