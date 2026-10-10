---@meta nsmbu.data
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.

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

return Data
