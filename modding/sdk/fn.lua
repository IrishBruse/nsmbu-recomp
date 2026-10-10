---@meta nsmbu.fn
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.

---@class nsmbu.Fn
---@field dScnPly_Execute nsmbu.GuestAddr|nil
---@field cLib_addCalc2 nsmbu.GuestAddr|nil
---@field example_coin_count nsmbu.GuestAddr|nil
---@field course_timer_tick nsmbu.GuestAddr|nil
---@field player_lose_life nsmbu.GuestAddr|nil
---@field player_lose_powerup nsmbu.GuestAddr|nil
---@field player_apply_jump nsmbu.GuestAddr|nil
---@field player_actor_ptr nsmbu.GuestAddr|nil
---@field player_begin_death nsmbu.GuestAddr|nil
---@field player_clear_damage nsmbu.GuestAddr|nil
---@field course_enter nsmbu.GuestAddr|nil
---@field checkpoint_touched nsmbu.GuestAddr|nil
---@field course_quit_to_map nsmbu.GuestAddr|nil
---@field [string] nsmbu.GuestAddr|nil
local Fn = {}

return Fn
