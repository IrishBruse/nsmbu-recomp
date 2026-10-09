#include "nsmbu_guest.h"

#include "nsmbu/functions.h"

static u32 steps, returns;

NSMBU_HOOK(NSMBU_ADDR_dScnPly_Execute, on_play_scene, (void* scene)) {
    int every = nsmbu_config_int("every", 60);
    if (every < 1) every = 1;
    if (++steps % (u32)every) return;
    nsmbu_log_int("play-scene-ticker: logic steps", (int)steps);
}

NSMBU_HOOK_RETURN(NSMBU_ADDR_dScnPly_Execute, after_play_scene, (void* scene)) {
    if (++returns == 1) nsmbu_log_hex("play-scene-ticker: first return hook, scene at", (u32)scene);
}
