const scene = nsmbu.scene
const hooks = nsmbu.hooks
const config = nsmbu.config

local done = false
local opened = false
local last = ""

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

  if opened then
    note(step, string.format("wait world map phase=%s", phase_text()))
    return
  end

  if not (scene.title() or scene.map_phase()) then
    note(step, string.format("wait boot phase=%s", phase_text()))
    return
  end

  local picked = config.string("save_file", "1")
  local save = 1
  if picked == "2" then
    save = 2
  elseif picked == "3" then
    save = 3
  end
  local ok, err = scene.open_first_save(save - 1)
  if ok == true then
    opened = true
    note(step, string.format("open save %d", save))
    return
  end
  note(step, string.format("open_first_save %s phase=%s", err or "refused", phase_text()))
end

function hooks.on_unload()
  done = false
  opened = false
  last = ""
end
