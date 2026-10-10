local steps = 0
local returns = 0

local function every_n()
  local every = nsmbu.config.number("every", 60)
  if every < 1 then
    return 1
  end
  return every
end

local entry_id, entry_err = nsmbu.hook(nsmbu.fn.dScnPly_Execute, "entry", function(ctx)
  steps = steps + 1
  if steps % every_n() ~= 0 then
    return
  end
  nsmbu.log_int("logic steps", steps)
end)

if not entry_id then
  nsmbu.log(entry_err)
end

local return_id, return_err = nsmbu.hook(nsmbu.fn.dScnPly_Execute, "return", function(ctx)
  returns = returns + 1
  if returns ~= 1 then
    return
  end
  nsmbu.log_hex("first return, r3", ctx.r3)
end)

if not return_id then
  nsmbu.log(return_err)
end

function nsmbu.on_unload()
  local written, err = nsmbu.data.write("steps.txt", tostring(steps))
  if not written then
    nsmbu.log(err)
  end
end
