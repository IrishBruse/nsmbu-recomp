local held_mode: { [number]: number? } = {}
local prev_speed_y: { [number]: number? } = {}
local scaled_jump: { [number]: boolean? } = {}
local last_parts: string = ""

local function freeze_timer(): boolean
  return nsmbu.level.set_time(500) == true
end

local function keep_lives(): boolean
  local ok = false
  for p in nsmbu.player.players() do
    local lives = p:lives()
    if lives and lives > 0 then
      if lives < 99 then
        p:set_lives(99)
      end
      ok = true
    end
  end
  return ok
end

local function keep_powerup(): boolean
  local ok = false
  for i = 0, 3 do
    local p = nsmbu.player(i)
    if not p or not p:entered() then
      held_mode[i] = nil
    else
      local mode = p:mode()
      if mode then
        local held = held_mode[i]
        if held == nil or mode > held then
          held_mode[i] = mode
          held = mode
        end
        if held and mode < held and held > 0 then
          p:set_mode(held)
        end
        ok = true
      end
    end
  end
  return ok
end

local function scale_jumps(): boolean
  local scale = nsmbu.config.number("jump_scale", 1)
  if not scale or scale == 1 then
    return false
  end
  local ok = false
  for i = 0, 3 do
    local p = nsmbu.player(i)

    if p ==nil then
      continue
    end

    local speed = p and p:speed_y()
    if not speed then
      prev_speed_y[i] = nil
      scaled_jump[i] = false
    else
      local prev = prev_speed_y[i] or 0
      if speed > 1.5 and prev <= 1.5 then
        scaled_jump[i] = false
      end
      if speed <= 0 then
        scaled_jump[i] = false
      elseif not scaled_jump[i] and speed > 1.5 then
        p:set_speed_y(speed * scale)
        scaled_jump[i] = true
      end
      prev_speed_y[i] = speed
      ok = true
    end
  end
  return ok
end

nsmbu.listen("logic_step", function(step: number)
  local parts: { string } = {}
  if nsmbu.config.bool("infinite_time", false) and freeze_timer() then
    parts[#parts + 1] = "time"
  end
  if nsmbu.config.bool("infinite_lives", false) and keep_lives() then
    parts[#parts + 1] = "lives"
  end
  if nsmbu.config.bool("keep_powerup", false) and keep_powerup() then
    parts[#parts + 1] = "power-up"
  end
  local scale = nsmbu.config.number("jump_scale", 1)
  if scale and scale ~= 1 and scale_jumps() then
    parts[#parts + 1] = "jump x" .. tostring(scale)
  end
  local line = table.concat(parts, ", ")
  if line ~= last_parts then
    last_parts = line
    if #parts == 0 then
      nsmbu.log("assists idle")
    else
      nsmbu.log("assists active: " .. line)
    end
  end
  if step % 60 ~= 0 then
    return
  end
  if #parts == 0 then
    nsmbu.status("Course assists off")
    return
  end
  nsmbu.status(line)
end)

nsmbu.listen("config_changed", function()
  held_mode = {}
  prev_speed_y = {}
  scaled_jump = {}
  last_parts = ""
end)
