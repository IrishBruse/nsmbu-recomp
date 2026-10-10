---@meta nsmbu
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.
--- Types are split one file per module.
--- Point the Lua language server at this directory, or open modding/examples.
--- There is no nsmbu global. Use require("nsmbu"), require("nsmbu.hooks"),
--- require("nsmbu.config"), and the other nsmbu.* modules.

---@alias nsmbu.GuestAddr integer
---@alias nsmbu.HookId integer
---@alias nsmbu.ListenId integer
---@alias nsmbu.HookWhen "entry"|"return"
---@alias nsmbu.ListenEvent "logic_step"|"config_changed"|"unload"

---@class nsmbu.Package
---@field id string Manifest package id.
---@field version string Manifest version.
---@field path string Absolute directory of the installed package.

---@alias nsmbu.LogicStepCallback fun(step: integer)
---@alias nsmbu.VoidCallback fun()

---@class nsmbu
---@field package nsmbu.Package
---@field on_logic_step nsmbu.LogicStepCallback|nil
---@field on_config_changed nsmbu.VoidCallback|nil
---@field on_unload nsmbu.VoidCallback|nil
local nsmbu = {}

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

return nsmbu
