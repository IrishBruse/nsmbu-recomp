

#include "ppc.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

enum { kLock = 0x10000010u };

__attribute__((noinline)) void spin_lock(Cpu* __restrict c) {
    c->r[31] = kLock;
L_02758940: ;
    c->r[10] = ld32(c->r[31] + 0x00000000u);
    cr_set_s(c, 0, (int32_t)c->r[10], 1);
    if (c->cr[2]) { PPC_LOOP(); goto L_02758940; }
L_0275894C: ;
    c->r[7] = ppc_lwarx(c, 0u + c->r[31]);
    cr_set_s(c, 0, (int32_t)c->r[7], 0);
    if (!c->cr[2]) { PPC_LOOP(); goto L_0275894C; }
    c->r[0] = 0u + 0x00000001u;
    ppc_stwcx(c, 0u + c->r[31], c->r[0]);
    if (!c->cr[2]) { PPC_LOOP(); goto L_0275894C; }
}

static void sleep_ms(int ms) {
    struct timespec ts = {ms / 1000, (long)(ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
}

static void* releaser(void* arg) {
    (void)arg;
    sleep_ms(50);
    __atomic_store_n((uint32_t*)ppc_ptr(kLock), 0u, __ATOMIC_SEQ_CST);
    return NULL;
}

static void* watchdog(void* arg) {
    (void)arg;
    sleep_ms(5000);
    fprintf(stderr, "spin_wait_test: FAIL, the spin wait never saw the lock released (stale load hoisted out of the loop)\n");
    _exit(1);
}

int main(void) {
    uint8_t* page = ppc_ptr(kLock & ~0xFFFFu);
    if (mmap(page, 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0) != page) {
        perror("spin_wait_test: mmap guest page");
        return 1;
    }
    st32(kLock, 1);
    pthread_t r, w;
    pthread_create(&w, NULL, watchdog, NULL);
    pthread_create(&r, NULL, releaser, NULL);
    static Cpu c;
    spin_lock(&c);
    pthread_join(r, NULL);
    if (ld32(kLock) != 1) {
        fprintf(stderr, "spin_wait_test: FAIL, lock not taken\n");
        return 1;
    }
    printf("spin_wait_test: ok\n");
    return 0;
}
