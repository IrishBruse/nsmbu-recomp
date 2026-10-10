---@meta nsmbu.player
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.

---@class nsmbu.Vec3
---@field x number
---@field y number
---@field z number

---@class nsmbu.PlayerHandle
---@field index integer 0..3
local PlayerHandle = {}

---@return boolean|nil entered
---@return string|nil err
function PlayerHandle:entered() end

---@return number|nil lives
---@return string|nil err
function PlayerHandle:lives() end

---@param lives number
---@return true|nil ok
---@return string|nil err
function PlayerHandle:set_lives(lives) end

---@return number|nil mode
---@return string|nil err
function PlayerHandle:mode() end

---@param mode number
---@return true|nil ok
---@return string|nil err
function PlayerHandle:set_mode(mode) end

---@return number|nil coins
---@return string|nil err
function PlayerHandle:coins() end

---@param coins number
---@return true|nil ok
---@return string|nil err
function PlayerHandle:set_coins(coins) end

---@return number|nil speed
---@return string|nil err
function PlayerHandle:speed_y() end

---@param speed number
---@return true|nil ok
---@return string|nil err
function PlayerHandle:set_speed_y(speed) end

---@return nsmbu.Vec3|nil pos
---@return string|nil err
function PlayerHandle:pos() end

---@param x number|nsmbu.Vec3
---@param y number|nil
---@param z number|nil
---@return true|nil ok
---@return string|nil err
function PlayerHandle:set_pos(x, y, z) end

---@return nsmbu.Vec3|nil speed
---@return string|nil err
function PlayerHandle:speed() end

---@param x number|nsmbu.Vec3
---@param y number|nil
---@param z number|nil
---@return true|nil ok
---@return string|nil err
function PlayerHandle:set_speed(x, y, z) end

---@return boolean|nil grounded
---@return string|nil err
function PlayerHandle:grounded() end

---@return nsmbu.GuestAddr|nil addr
---@return string|nil err
function PlayerHandle:actor() end

---@class nsmbu.PlayerModule
local PlayerModule = {}

---@param index integer 0..3
---@return nsmbu.PlayerHandle|nil player
---@return string|nil err
function PlayerModule.get(index) end

---@param index integer 0..3
---@return nsmbu.PlayerHandle|nil player
---@return string|nil err
function PlayerModule:__call(index) end

---@return fun(): nsmbu.PlayerHandle|nil
function PlayerModule.players() end

return PlayerModule
