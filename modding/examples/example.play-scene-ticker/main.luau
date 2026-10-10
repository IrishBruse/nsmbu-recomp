local nsmbu = require("nsmbu")
local config = require("nsmbu.config")
local fn = require("nsmbu.fn")
local hooks = require("nsmbu.hooks")
local data = require("nsmbu.data")

local steps = 0
local returns = 0

local function every_n(): number
  local every = config.number("every", 60)
  if every < 1 then
    return 1
  end
  return every
end

local target = fn.dScnPly_Execute
if target and hooks.hook then
  local entry_id, entry_err = hooks.hook(target, "entry", function(ctx: any)
    steps = steps + 1
    if steps % every_n() ~= 0 then
      return
    end
    nsmbu.log_int("logic steps", steps)
  end)

  if not entry_id then
    nsmbu.log(entry_err)
  end

  local return_id, return_err = hooks.hook(target, "return", function(ctx: any)
    returns = returns + 1
    if returns ~= 1 then
      return
    end
    nsmbu.log_hex("first return, r3", ctx.r3)
  end)

  if not return_id then
    nsmbu.log(return_err)
  end
else
  nsmbu.log("dScnPly_Execute or hooks.hook is not in nsmbu")
end

function nsmbu.on_unload()
  if not data.write then
    nsmbu.log("data.write is not in nsmbu.data")
    return
  end
  local written, err = data.write("steps.txt", tostring(steps))
  if not written then
    nsmbu.log(err)
  end
end
