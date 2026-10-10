---@meta nsmbu.config
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.

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

return Config
