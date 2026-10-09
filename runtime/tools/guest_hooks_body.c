

#include "ppc.h"

void guest_hooks_record_body(void);

void guest_hooks_body(Cpu* __restrict c) {
    PPC_ENTER(0x02000000u);
    PPC_MOD_HOOK(0, 0x02000000u);
    guest_hooks_record_body();
    c->r[3] += 1;
}
