---@meta
--- Proposed NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md. The runtime does not implement this yet.
--- Point the Lua language server library path at this file, or open modding/examples.

---@alias nsmbu.GuestAddr integer
---@alias nsmbu.HookId integer
---@alias nsmbu.ListenId integer
---@alias nsmbu.HookWhen "entry"|"return"
---@alias nsmbu.ListenEvent "logic_step"|"config_changed"|"unload"
---@alias nsmbu.ButtonName
---| "A" | "B" | "X" | "Y"
---| "L" | "R" | "ZL" | "ZR"
---| "Plus" | "Minus" | "Home"
---| "Up" | "Down" | "Left" | "Right"
---| "StickL" | "StickR"

---@class nsmbu.Package
---@field id string Manifest package id.
---@field version string Manifest version.
---@field path string Absolute directory of the installed package.

---@class nsmbu.Config
local Config = {}

---@param id string
---@param fallback string
---@return string|nil value
---@return string|nil err
function Config.string(id, fallback) end

---@param id string
---@param fallback number
---@return number|nil value
---@return string|nil err
function Config.number(id, fallback) end

---@param id string
---@param fallback boolean
---@return boolean|nil value
---@return string|nil err
function Config.bool(id, fallback) end

---@class nsmbu.Guest
local Guest = {}

---@param addr nsmbu.GuestAddr
---@return integer|nil value 0..255
---@return string|nil err
function Guest.read_u8(addr) end

---@param addr nsmbu.GuestAddr
---@return integer|nil value -128..127
---@return string|nil err
function Guest.read_s8(addr) end

---@param addr nsmbu.GuestAddr
---@return integer|nil value 0..65535
---@return string|nil err
function Guest.read_u16(addr) end

---@param addr nsmbu.GuestAddr
---@return integer|nil value -32768..32767
---@return string|nil err
function Guest.read_s16(addr) end

---@param addr nsmbu.GuestAddr
---@return integer|nil value 0..4294967295
---@return string|nil err
function Guest.read_u32(addr) end

---@param addr nsmbu.GuestAddr
---@return integer|nil value
---@return string|nil err
function Guest.read_s32(addr) end

---@param addr nsmbu.GuestAddr
---@return number|nil value
---@return string|nil err
function Guest.read_f32(addr) end

---@param addr nsmbu.GuestAddr
---@return number|nil value
---@return string|nil err
function Guest.read_f64(addr) end

---@param addr nsmbu.GuestAddr
---@param value integer
---@return true|nil ok
---@return string|nil err
function Guest.write_u8(addr, value) end

---@param addr nsmbu.GuestAddr
---@param value integer
---@return true|nil ok
---@return string|nil err
function Guest.write_s8(addr, value) end

---@param addr nsmbu.GuestAddr
---@param value integer
---@return true|nil ok
---@return string|nil err
function Guest.write_u16(addr, value) end

---@param addr nsmbu.GuestAddr
---@param value integer
---@return true|nil ok
---@return string|nil err
function Guest.write_s16(addr, value) end

---@param addr nsmbu.GuestAddr
---@param value integer
---@return true|nil ok
---@return string|nil err
function Guest.write_u32(addr, value) end

---@param addr nsmbu.GuestAddr
---@param value integer
---@return true|nil ok
---@return string|nil err
function Guest.write_s32(addr, value) end

---@param addr nsmbu.GuestAddr
---@param value number
---@return true|nil ok
---@return string|nil err
function Guest.write_f32(addr, value) end

---@param addr nsmbu.GuestAddr
---@param value number
---@return true|nil ok
---@return string|nil err
function Guest.write_f64(addr, value) end

---@param addr nsmbu.GuestAddr
---@param size integer
---@return string|nil bytes
---@return string|nil err
function Guest.read(addr, size) end

---@param addr nsmbu.GuestAddr
---@param data string
---@return true|nil ok
---@return string|nil err
function Guest.write(addr, data) end

---@param size integer
---@return nsmbu.GuestAddr|nil addr
---@return string|nil err
function Guest.alloc(size) end

---@param addr nsmbu.GuestAddr
---@return true|nil ok
---@return string|nil err
function Guest.free(addr) end

--- Generated guest function addresses for the running build.
--- Missing names are nil until the header export includes them.
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

---@class nsmbu.HookCtx
---@field address nsmbu.GuestAddr Read-only guest address of the hooked function.
---@field r3 integer
---@field r4 integer
---@field r5 integer
---@field r6 integer
---@field r7 integer
---@field r8 integer
---@field r9 integer
---@field r10 integer
---@field f1 number
---@field f2 number
---@field f3 number
---@field f4 number
---@field f5 number
---@field f6 number
---@field f7 number
---@field f8 number
---@field skip boolean|nil Entry hooks only. true skips the body, replacement, and later entry hooks.

---@alias nsmbu.HookCallback fun(ctx: nsmbu.HookCtx)

---@class nsmbu.CallArgs
---@field r3 integer|nil
---@field r4 integer|nil
---@field r5 integer|nil
---@field r6 integer|nil
---@field r7 integer|nil
---@field r8 integer|nil
---@field r9 integer|nil
---@field r10 integer|nil
---@field f1 number|nil
---@field f2 number|nil
---@field f3 number|nil
---@field f4 number|nil
---@field f5 number|nil
---@field f6 number|nil
---@field f7 number|nil
---@field f8 number|nil

---@class nsmbu.CallResult
---@field r3 integer
---@field r4 integer
---@field f1 number

---@class nsmbu.Pad
---@field buttons integer Bit mask; see nsmbu.button.
---@field lx number
---@field ly number
---@field rx number
---@field ry number
---@field touch boolean
---@field tx number
---@field ty number

---@class nsmbu.Button
---@field A integer
---@field B integer
---@field X integer
---@field Y integer
---@field L integer
---@field R integer
---@field ZL integer
---@field ZR integer
---@field Plus integer
---@field Minus integer
---@field Home integer
---@field Up integer
---@field Down integer
---@field Left integer
---@field Right integer
---@field StickL integer
---@field StickR integer
local Button = {}

---@param name nsmbu.ButtonName
---@return boolean|nil down
---@return string|nil err
function Button.held(name) end

---@param name nsmbu.ButtonName
---@return boolean|nil pressed
---@return string|nil err
function Button.pressed(name) end

---@class nsmbu.Data
local Data = {}

---@param name string One path segment under ModManager/Data/<id>/.
---@return string|nil bytes
---@return string|nil err
function Data.read(name) end

---@param name string
---@param data string
---@return integer|nil bytes_written
---@return string|nil err
function Data.write(name, data) end

---@param name string
---@return true|nil ok
---@return string|nil err
function Data.remove(name) end

---@alias nsmbu.LogicStepCallback fun(step: integer)
---@alias nsmbu.VoidCallback fun()

---@class nsmbu
---@field package nsmbu.Package
---@field config nsmbu.Config
---@field guest nsmbu.Guest
---@field fn nsmbu.Fn
---@field button nsmbu.Button
---@field data nsmbu.Data
---@field on_logic_step nsmbu.LogicStepCallback|nil
---@field on_config_changed nsmbu.VoidCallback|nil
---@field on_unload nsmbu.VoidCallback|nil
nsmbu = {}

---@param message string
---@return true|nil ok
---@return string|nil err
function nsmbu.log(message) end

---@param label string
---@param value integer
---@return true|nil ok
---@return string|nil err
function nsmbu.log_int(label, value) end

---@param label string
---@param value integer
---@return true|nil ok
---@return string|nil err
function nsmbu.log_hex(label, value) end

---@param label string
---@param value number
---@return true|nil ok
---@return string|nil err
function nsmbu.log_float(label, value) end

---@param message string
---@return true|nil ok
---@return string|nil err
function nsmbu.status(message) end

---@return integer step
function nsmbu.logic_step() end

---@return number seconds
function nsmbu.logic_dt() end

---@param event nsmbu.ListenEvent
---@param callback nsmbu.LogicStepCallback|nsmbu.VoidCallback
---@return nsmbu.ListenId|nil id
---@return string|nil err
function nsmbu.listen(event, callback) end

---@param id nsmbu.ListenId
---@return true|nil ok
---@return string|nil err
function nsmbu.unlisten(id) end

---@param target nsmbu.GuestAddr
---@param when nsmbu.HookWhen
---@param callback nsmbu.HookCallback
---@return nsmbu.HookId|nil id
---@return string|nil err
function nsmbu.hook(target, when, callback) end

---@param target nsmbu.GuestAddr
---@param callback nsmbu.HookCallback
---@return nsmbu.HookId|nil id
---@return string|nil err
function nsmbu.replace(target, callback) end

---@param id nsmbu.HookId
---@return true|nil ok
---@return string|nil err
function nsmbu.unhook(id) end

--- Valid only inside a replace callback.
---@return true|nil ok
---@return string|nil err
function nsmbu.original() end

---@param target nsmbu.GuestAddr
---@param args nsmbu.CallArgs|nil
---@return true|nil ok
---@return string|nil err
function nsmbu.call(target, args) end

---@param target nsmbu.GuestAddr
---@param args nsmbu.CallArgs|nil
---@return nsmbu.CallResult|nil result
---@return string|nil err
function nsmbu.call_result(target, args) end

---@return nsmbu.Pad
function nsmbu.input() end
