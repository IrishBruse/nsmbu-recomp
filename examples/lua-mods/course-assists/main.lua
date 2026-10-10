local symbols = {
  course_timer_tick = "infinite_time",
  player_lose_life = "infinite_lives",
  player_lose_powerup = "keep_powerup",
}

local function hook_skip(symbol, option_id)
  local target = nsmbu.fn[symbol]
  if not target then
    nsmbu.log(symbol .. " is not in nsmbu.fn")
    return
  end
  local id, err = nsmbu.hook(target, "entry", function(ctx)
    if nsmbu.config.bool(option_id, false) then
      ctx.skip = true
    end
  end)
  if not id then
    nsmbu.log(err)
  end
end

for symbol, option_id in pairs(symbols) do
  hook_skip(symbol, option_id)
end

local jump = nsmbu.fn.player_apply_jump
if not jump then
  nsmbu.log("player_apply_jump is not in nsmbu.fn")
else
  local id, err = nsmbu.hook(jump, "entry", function(ctx)
    local scale = nsmbu.config.number("jump_scale", 1)
    if scale == 1 then
      return
    end
    ctx.f1 = ctx.f1 * scale
  end)
  if not id then
    nsmbu.log(err)
  end
end

function nsmbu.on_logic_step(step)
  if step % 60 ~= 0 then
    return
  end
  local parts = {}
  if nsmbu.config.bool("infinite_time", false) then
    parts[#parts + 1] = "time"
  end
  if nsmbu.config.bool("infinite_lives", false) then
    parts[#parts + 1] = "lives"
  end
  if nsmbu.config.bool("keep_powerup", false) then
    parts[#parts + 1] = "power-up"
  end
  local scale = nsmbu.config.number("jump_scale", 1)
  if scale ~= 1 then
    parts[#parts + 1] = "jump x" .. tostring(scale)
  end
  if #parts == 0 then
    nsmbu.status("Course assists off")
    return
  end
  nsmbu.status(table.concat(parts, ", "))
end
