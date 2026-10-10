---@meta nsmbu.hooks
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.

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

---@class nsmbu.Hook
local Hook = {}

---@param target nsmbu.GuestAddr
---@param when nsmbu.HookWhen
---@param callback nsmbu.HookCallback
---@return nsmbu.HookId|nil id
---@return string|nil err
function Hook.hook(target, when, callback) end

---@param target nsmbu.GuestAddr
---@param callback nsmbu.HookCallback
---@return nsmbu.HookId|nil id
---@return string|nil err
function Hook.replace(target, callback) end

---@param id nsmbu.HookId
---@return true|nil ok
---@return string|nil err
function Hook.unhook(id) end

---@return true|nil ok
---@return string|nil err
function Hook.original() end

---@param target nsmbu.GuestAddr
---@param args nsmbu.CallArgs|nil
---@return true|nil ok
---@return string|nil err
function Hook.call(target, args) end

---@param target nsmbu.GuestAddr
---@param args nsmbu.CallArgs|nil
---@return nsmbu.CallResult|nil result
---@return string|nil err
function Hook.call_result(target, args) end

return Hook
