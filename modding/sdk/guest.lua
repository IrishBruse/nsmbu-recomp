---@meta nsmbu.guest
--- NSMBU Lua API v1 type declarations.
--- Matches docs/modding/api.md.

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

return Guest
