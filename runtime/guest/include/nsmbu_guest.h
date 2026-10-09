#pragma once

static const unsigned int nsmbu_address_format __attribute__((used, section(".nsmbu_addresses"))) = 1;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
typedef double f64;

#define NSMBU_GUEST_API_VERSION 1

typedef struct { u32 kind, target; void* func; u32 flags; } nsmbu_hook_desc;
#define NSMBU_KIND_REPLACE 1
#define NSMBU_KIND_ENTRY 2
#define NSMBU_KIND_RETURN 3
#define NSMBU__CAT2(a, b) a##b
#define NSMBU__CAT(a, b) NSMBU__CAT2(a, b)
#define NSMBU__DESC(kind, addr, fn)                                                                   \
    __attribute__((section(".nsmbu_hooks"), used)) static const nsmbu_hook_desc NSMBU__CAT(nsmbu__h_, fn) = \
        {kind, addr, (void*)&fn, 0}

#define NSMBU_REPLACE(addr, ret, name, params) \
    static ret name params;                   \
    NSMBU__DESC(NSMBU_KIND_REPLACE, addr, name); \
    __attribute__((noinline, used)) static ret name params

#define NSMBU_HOOK(addr, name, params) \
    static void name params;          \
    NSMBU__DESC(NSMBU_KIND_ENTRY, addr, name); \
    __attribute__((noinline, used)) static void name params
#define NSMBU_HOOK_RETURN(addr, name, params) \
    static void name params;                 \
    NSMBU__DESC(NSMBU_KIND_RETURN, addr, name); \
    __attribute__((noinline, used)) static void name params

#define NSMBU__STR2(x) #x
#define NSMBU__STR(x) NSMBU__STR2(x)
#define NSMBU_GAME_FUNC(addr, ret, name, params) ret name params __asm__("__nsmbu_game_" NSMBU__STR(addr))
#define NSMBU_GAME_ORIGINAL(addr, ret, name, params) ret name params __asm__("__nsmbu_orig_" NSMBU__STR(addr))
#define NSMBU_GAME_DATA(addr, type) (*({ \
    extern u8 NSMBU__CAT(nsmbu_game_data_, addr)[] __asm__("__nsmbu_gdata_" NSMBU__STR(addr)); \
    (type volatile*)NSMBU__CAT(nsmbu_game_data_, addr); }))

void nsmbu_log(const char* message);
void nsmbu_log_int(const char* label, int value);
void nsmbu_log_hex(const char* label, u32 value);
void nsmbu_log_float(const char* label, double value);
int nsmbu_config_int(const char* id, int fallback);
int nsmbu_config_bool(const char* id, int fallback);
double nsmbu_config_float(const char* id, double fallback);
u32 nsmbu_config_string(const char* id, char* buffer, u32 capacity);
void* nsmbu_malloc(u32 size);
void nsmbu_free(void* pointer);
typedef struct {
    u32 buttons;
    f32 lx, ly, rx, ry;
    u32 touch;
    f32 tx, ty;
} nsmbu_input_state;
void nsmbu_input_read(nsmbu_input_state* state);
s32 nsmbu_file_read(const char* filename, void* buffer, u32 size);
s32 nsmbu_file_write(const char* filename, const void* buffer, u32 size);
double nsmbu_logic_dt(void);
unsigned long long nsmbu_logic_step(void);
void* memcpy(void* dst, const void* src, unsigned long n);
void* memmove(void* dst, const void* src, unsigned long n);
void* memset(void* dst, int v, unsigned long n);
