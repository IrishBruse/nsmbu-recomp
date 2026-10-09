
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define NSMBU_MOD_ABI_VERSION 1
#if defined(_WIN32)
#define NSMBU_MOD_EXPORT __declspec(dllexport)
#else
#define NSMBU_MOD_EXPORT __attribute__((visibility("default")))
#endif
typedef struct NSMBUModHostV1 {
    uint32_t size, abi_version;
    void* context;
    const char* game_id;
    const char* package_dir;

    int (*read_guest)(void*, uint32_t address, void* output, size_t size);
    int (*write_guest)(void*, uint32_t address, const void* input, size_t size);
    void (*log)(void*, const char* message);
    void (*status)(void*, const char* message);
    const char* (*get_string)(void*, const char* option_id);
    double (*get_number)(void*, const char* option_id);
    int (*get_bool)(void*, const char* option_id);
} NSMBUModHostV1;
typedef struct NSMBUModV1 {
    uint32_t size, abi_version;
    void* instance;

    void (*on_frame)(void*, uint64_t logic_step);
    void (*on_config_changed)(void*);

    void (*on_unload)(void*);
} NSMBUModV1;
typedef int (*NSMBUModInitV1)(const NSMBUModHostV1*, NSMBUModV1*);

NSMBU_MOD_EXPORT int nsmbu_mod_init_v1(const NSMBUModHostV1*, NSMBUModV1*);
#ifdef __cplusplus
}
#endif
