const scene = nsmbu.scene
const hooks = nsmbu.hooks

local stage = 0
local started = false
local last = ""

local function accepted(ok: boolean?): boolean
  return ok == true
end

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
  if stage == 0 then
    if started or scene.title() or scene.map_phase() then
      local opened, err = scene.open_file_select()
      if accepted(opened) then
        note(step, "save slot page")
        stage = 1
      else
        started = true
        note(step, string.format("open_file_select %s phase=%s", err or "refused", phase_text()))
      end
    else
      note(step, string.format("wait title phase=%s", phase_text()))
    end
  elseif stage == 1 then
    if scene.save_select() then
      note(step, "unexpected challenge select")
      return
    end
    local file, file_err = scene.file_index()
    if file == 0 then
      note(step, "save slot 0")
      stage = 4
    else
      note(step, string.format("wait file_index %s phase=%s", file_err or "nil", phase_text()))
    end
  end
end

function hooks.on_unload()
  stage = 0
  started = false
  last = ""
end
