const scene = nsmbu.scene
const input = nsmbu.input
const hooks = nsmbu.hooks

local AFTER_TITLE = 0x0202B6C4
local PRE_COURSE = 0x0202B570
local COURSE_SELECT = 0x0202B86C

local done = false
local can_press = false
local last = ""
local last_press = -1

local function note(step: number, message: string)
  if message == last then
    return
  end
  last = message
  nsmbu.log(string.format("intro-skip step %d %s", step, message))
end

local function phase_text(): string
  local phase = scene.phase()
  if phase == nil then
    return "none"
  end
  return string.format("%08X", phase)
end

function hooks.on_logic_step(step: number)
  if done then
    return
  end

  if scene.save_select() then
    note(step, "unexpected challenge select")
    return
  end

  if scene.course_select() then
    note(step, "world map")
    done = true
    return
  end

  local phase = scene.phase()
  if phase == COURSE_SELECT then
    note(step, "world map")
    done = true
    return
  end

  if scene.title() or scene.map_phase() or phase == AFTER_TITLE or phase == PRE_COURSE then
    can_press = true
  end

  if not can_press then
    note(step, string.format("wait boot phase=%s", phase_text()))
    return
  end

  if last_press >= 0 and step - last_press < 45 then
    return
  end
  last_press = step
  input.press("A", 12)
  note(step, string.format("press A phase=%s", phase_text()))
end

function hooks.on_unload()
  done = false
  can_press = false
  last = ""
  last_press = -1
end
