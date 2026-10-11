local steps = 0
local returns = 0

local function every_n(): number
  local every = nsmbu.config.number("every", 60)
  if not every or every < 1 then
    return 1
  end
  return every
end

local target = nsmbu.fn.dScnPly_Execute
if target and nsmbu.hooks.hook then
  local entry_id, entry_err = nsmbu.hooks.hook(target, "entry", function(ctx)
    steps = steps + 1
    if steps % every_n() ~= 0 then
      return
    end
    nsmbu.log_int("logic steps", steps)
  end)

  if not entry_id then
    nsmbu.log(entry_err or "entry hook failed")
  end

  local return_id, return_err = nsmbu.hooks.hook(target, "return", function(ctx)
    returns = returns + 1
    if returns ~= 1 then
      return
    end
    nsmbu.log_hex("first return, r3", ctx.r3)
  end)

  if not return_id then
    nsmbu.log(return_err or "return hook failed")
  end
else
  nsmbu.log("dScnPly_Execute or nsmbu.hooks.hook is not in nsmbu")
end

nsmbu.listen("unload", function()
  if not nsmbu.data.write then
    nsmbu.log("nsmbu.data.write is not in nsmbu.data")
    return
  end
  local written, err = nsmbu.data.write("steps.txt", tostring(steps))
  if not written then
    nsmbu.log(err or "data write failed")
  end
end)
