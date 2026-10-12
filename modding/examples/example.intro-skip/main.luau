const config = nsmbu.config
const scene = nsmbu.scene
const guest = nsmbu.guest
const press = nsmbu.input.press

local kCourseInfo = 0x101D18EC

local cfg = {}
local next_press = 0

local function read_config()
  cfg = {
    enabled = config.bool("enabled", false),
    every = config.number("press_every", 20),
    hold = config.number("press_hold", 3),
    debug = config.bool("debug", false),
  }
end

local function course_info()
  local ptr = guest.read_u32(kCourseInfo)
  if not ptr or ptr == 0 then
    return "-"
  end
  local now_file = guest.read_u8(ptr + 0x2)
  local next_world = guest.read_u8(ptr + 0x10)
  local next_file = guest.read_u8(ptr + 0x12)
  return string.format("now_file=%d next_w=%d next_f=%d", now_file or -1, next_world or -1,
    next_file or -1)
end

nsmbu.on_logic_step(function(step)
  if step % 15 == 0 then
    read_config()
  end
  if not cfg.enabled then
    return
  end
  if cfg.debug and step % 15 == 0 then
    nsmbu.log(string.format("step %d wm=%s course=%s info=%s", step,
      tostring(scene.world_map()), tostring(scene.course()), course_info()))
  end
  if scene.course() then
    if next_press ~= 0 then
      nsmbu.log(string.format("course scene at %d, stopping presses", step))
    end
    next_press = 0
    return
  end
  if scene.world_map() then
    return
  end
  if step >= next_press then
    press("A", cfg.hold)
    if cfg.debug then
      nsmbu.log(string.format("press A at %d", step))
    end
    next_press = step + cfg.every
  end
end)

nsmbu.on_config_changed(read_config)
nsmbu.on_unload(function()
  next_press = 0
end)
read_config()
