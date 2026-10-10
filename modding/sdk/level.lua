---@meta nsmbu.level
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.

---@class nsmbu.Level
local Level = {}

---@return boolean
function Level.active() end

---@return number|nil seconds
---@return string|nil err
function Level.time() end

---@param seconds number
---@return true|nil ok
---@return string|nil err
function Level.set_time(seconds) end

---@param seconds number
---@return true|nil ok
---@return string|nil err
function Level.add_time(seconds) end

---@return number|nil coins
---@return string|nil err
function Level.coins() end

---@param coins number
---@return true|nil ok
---@return string|nil err
function Level.set_coins(coins) end

---@return number|nil score
---@return string|nil err
function Level.score() end

---@param score number
---@return true|nil ok
---@return string|nil err
function Level.set_score(score) end

return Level
