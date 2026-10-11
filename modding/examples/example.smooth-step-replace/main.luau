const fn = nsmbu.fn
const hooks = nsmbu.hooks
const guest = nsmbu.guest

local smooth = require("smooth")
local calls = 0
local own = 0

local target = fn.cLib_addCalc2
if not target then
  nsmbu.log("cLib_addCalc2 is not in fn")
  return
end

local id, err = hooks.replace(target, function(ctx)
  calls = calls + 1
  if calls % 65536 == 1 then
    nsmbu.log_int("calls", calls)
    nsmbu.log_int("handled by the mod", own)
  end
  if calls % 2 == 1 then
    hooks.original()
    return
  end
  local value, read_err = guest.read_f32(ctx.r3)
  if not value then
    nsmbu.log(read_err or "guest read failed")
    hooks.original()
    return
  end
  own = own + 1
  local next_value = smooth.step(value, ctx.f1, ctx.f2, ctx.f3)
  local wrote, write_err = guest.write_f32(ctx.r3, next_value)
  if not wrote then
    nsmbu.log(write_err or "guest write failed")
  end
end)

if not id then
  nsmbu.log(err or "replace failed")
end
