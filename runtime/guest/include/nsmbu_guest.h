#pragma once

#include "wwhd_guest.h"

#define NSMBU_GUEST_API_VERSION WWHD_GUEST_API_VERSION

#define NSMBU_KIND_REPLACE WWHD_KIND_REPLACE
#define NSMBU_KIND_ENTRY WWHD_KIND_ENTRY
#define NSMBU_KIND_RETURN WWHD_KIND_RETURN

#define NSMBU_REPLACE WWHD_REPLACE
#define NSMBU_HOOK WWHD_HOOK
#define NSMBU_HOOK_RETURN WWHD_HOOK_RETURN
#define NSMBU_GAME_FUNC WWHD_GAME_FUNC
#define NSMBU_GAME_ORIGINAL WWHD_GAME_ORIGINAL
#define NSMBU_GAME_DATA WWHD_GAME_DATA

#define nsmbu_log wwhd_log
#define nsmbu_log_int wwhd_log_int
#define nsmbu_log_hex wwhd_log_hex
#define nsmbu_log_float wwhd_log_float
#define nsmbu_config_int wwhd_config_int
#define nsmbu_config_bool wwhd_config_bool
#define nsmbu_config_float wwhd_config_float
#define nsmbu_config_string wwhd_config_string
#define nsmbu_malloc wwhd_malloc
#define nsmbu_free wwhd_free
#define nsmbu_input_state wwhd_input_state
#define nsmbu_input_read wwhd_input_read
#define nsmbu_file_read wwhd_file_read
#define nsmbu_file_write wwhd_file_write
#define nsmbu_logic_dt wwhd_logic_dt
#define nsmbu_logic_step wwhd_logic_step
