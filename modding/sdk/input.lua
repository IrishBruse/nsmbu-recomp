---@meta nsmbu.input
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.

---@alias nsmbu.ButtonName
---| "A" | "B" | "X" | "Y"
---| "L" | "R" | "ZL" | "ZR"
---| "Plus" | "Minus" | "Home"
---| "Up" | "Down" | "Left" | "Right"
---| "StickL" | "StickR"

---@class nsmbu.Pad
---@field buttons integer Bit mask; see input.button.
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

---@class nsmbu.Input
---@field button nsmbu.Button
local Input = {}

---@return nsmbu.Pad
function Input:__call() end

return Input
