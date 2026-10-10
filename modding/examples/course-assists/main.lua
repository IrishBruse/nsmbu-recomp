local COURSE_TIMER = 0x101D15F4
local FIELD_GAME = 0x101D1604
local PLAYER_MGR = 0x101E6994

local TIMER_TIME = 0x14
local PLAYER_DATA0 = 0x24
local PLAYER_DATA_SIZE = 0x48
local LIFE_OFF = 0x4
local MODE_OFF = 0x8
local PLAYER_OBJECTS = 0x20
local SPEED_Y = 0x7C
local ACTOR_MODE = 0x500

local TIME_CAP = 999 * 4096
local LIFE_CAP = 99

local held_mode = {}
local prev_speed_y = {}
local scaled_jump = {}
local last_parts = ""

local function guest_u32(addr)
  return nsmbu.guest.read_u32(addr)
end

local function instance(addr)
  local p = guest_u32(addr)
  if not p or p == 0 or p < 0x10000000 or p >= 0x50000000 then
    return nil
  end
  return p
end

local function freeze_timer()
  local timer = instance(COURSE_TIMER)
  if not timer then
    return false
  end
  local now = guest_u32(timer + TIMER_TIME)
  if not now then
    return false
  end
  if now < TIME_CAP then
    return nsmbu.guest.write_u32(timer + TIMER_TIME, TIME_CAP) == true
  end
  return true
end

local function player_slot(index)
  local field = instance(FIELD_GAME)
  if not field then
    return nil
  end
  return field + PLAYER_DATA0 + index * PLAYER_DATA_SIZE
end

local function keep_lives()
  local ok = false
  for i = 0, 3 do
    local slot = player_slot(i)
    if slot then
      local entry = nsmbu.guest.read_u8(slot)
      if entry and entry ~= 0 then
        local life = nsmbu.guest.read_s32(slot + LIFE_OFF)
        if life and life > 0 and life < LIFE_CAP then
          nsmbu.guest.write_s32(slot + LIFE_OFF, LIFE_CAP)
        end
        if life and life > 0 then
          ok = true
        end
      end
    end
  end
  return ok
end

local function player_actor(index)
  local mgr = instance(PLAYER_MGR)
  if not mgr then
    return nil
  end
  local actor = guest_u32(mgr + PLAYER_OBJECTS + index * 4)
  if not actor or actor < 0x10000000 or actor >= 0x50000000 then
    return nil
  end
  return actor
end

local function keep_powerup()
  local ok = false
  for i = 0, 3 do
    local slot = player_slot(i)
    if slot then
      local entry = nsmbu.guest.read_u8(slot)
      if not entry or entry == 0 then
        held_mode[i] = nil
      else
        local mode = nsmbu.guest.read_s32(slot + MODE_OFF)
        local actor = player_actor(i)
        if actor then
          local actor_mode = nsmbu.guest.read_s32(actor + ACTOR_MODE)
          if actor_mode and actor_mode >= 0 and actor_mode <= 8 then
            mode = actor_mode
          end
        end
        if mode and mode >= 0 and mode <= 8 then
          local held = held_mode[i]
          if held == nil or mode > held then
            held_mode[i] = mode
            held = mode
          end
          if held and mode < held and held > 0 then
            nsmbu.guest.write_s32(slot + MODE_OFF, held)
            if actor then
              nsmbu.guest.write_s32(actor + ACTOR_MODE, held)
            end
          end
          ok = true
        end
      end
    end
  end
  return ok
end

local function scale_jumps()
  local scale = nsmbu.config.number("jump_scale", 1)
  if not scale or scale == 1 then
    return false
  end
  local ok = false
  for i = 0, 3 do
    local actor = player_actor(i)
    if not actor then
      prev_speed_y[i] = nil
      scaled_jump[i] = false
    else
      local speed = nsmbu.guest.read_f32(actor + SPEED_Y)
      if speed then
        local prev = prev_speed_y[i] or 0
        if speed > 1.5 and prev <= 1.5 then
          scaled_jump[i] = false
        end
        if speed <= 0 then
          scaled_jump[i] = false
        elseif not scaled_jump[i] and speed > 1.5 then
          nsmbu.guest.write_f32(actor + SPEED_Y, speed * scale)
          scaled_jump[i] = true
        end
        prev_speed_y[i] = speed
        ok = true
      end
    end
  end
  return ok
end

function nsmbu.on_logic_step(step)
  local parts = {}
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
end

function nsmbu.on_config_changed()
  held_mode = {}
  prev_speed_y = {}
  scaled_jump = {}
  last_parts = ""
end
