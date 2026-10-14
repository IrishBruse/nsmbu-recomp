const scene = nsmbu.scene
const hooks = nsmbu.hooks

local stage = 0
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
    if scene.title() or scene.map_phase() then
      local opened, err = scene.open_file_select()
      if accepted(opened) then
        note(step, "open_file_select accepted")
        stage = 1
      else
        note(step, string.format("open_file_select %s", err or "refused"))
      end
    else
      note(step, string.format("wait title phase=%s", phase_text()))
    end
  elseif stage == 1 then
    if scene.save_select() then
      local file, file_err = scene.file_index()
      if file ~= nil then
        local confirmed, err = scene.confirm_file()
        if accepted(confirmed) then
          note(step, string.format("confirm_file accepted file=%d", file))
          stage = 2
        else
          note(step, string.format("confirm_file %s file=%d", err or "refused", file))
        end
      else
        note(step, string.format("wait file_index %s", file_err or "nil"))
      end
    else
      note(step, string.format("wait save_select phase=%s", phase_text()))
    end
  elseif stage == 2 then
    if scene.course_select() then
      note(step, "course_select already up")
      stage = 3
    elseif scene.course() and scene.course_task() then
      local opened, err = scene.open_course_select()
      if accepted(opened) then
        note(step, "open_course_select accepted")
        stage = 3
      else
        note(step, string.format("open_course_select %s", err or "refused"))
      end
    else
      note(
        step,
        string.format("wait course=%s course_task=%s phase=%s", tostring(scene.course()), tostring(scene.course_task()), phase_text())
      )
    end
  elseif stage == 3 then
    if scene.course_select() then
      note(step, "world map")
      stage = 4
    else
      note(step, string.format("wait course_select phase=%s", phase_text()))
    end
  end
end

function hooks.on_unload()
  stage = 0
  last = ""
end
