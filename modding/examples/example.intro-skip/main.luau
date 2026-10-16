const scene = nsmbu.scene
const hooks = nsmbu.hooks
const config = nsmbu.config

local k_boot = 0x0202930C

local done = false
local opened = false
local requested = false
local last = ""
local file_select_steps = 0
local popup_pulses = 0
local press_err_logged = false

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

  local pack = config.string("course_pack", "1-1") or "1-1"
  if pack == "" then
    pack = "1-1"
  end

  if scene.save_select() then
    note(step, "unexpected challenge select")
    return
  end

  if opened then
    if scene.course_task() then
      note(step, "course up")
      done = true
      return
    end
    if requested then
      note(step, string.format("wait course phase=%s", phase_text()))
      return
    end
    local ok, err = scene.load_pack(pack)
    if ok == true then
      requested = true
      note(step, string.format("load_pack %s", pack))
    else
      note(step, string.format("load_pack %s %s", pack, err or "refused"))
    end
    return
  end

  if scene.phase() == k_boot then
    note(step, "wait boot")
    return
  end

  local picked = config.string("save_file", "1")
  local save = 1
  if picked == "2" then
    save = 2
  elseif picked == "3" then
    save = 3
  end
  local ok, err = scene.apply_save(save - 1)
  if ok == true then
    opened = true
    note(step, string.format("apply save %d", save))
    return
  end

  if scene.file_select() and err == "file not ready" then
    file_select_steps += 1
    if popup_pulses < 4 then
      if file_select_steps % 30 == 0 then
        popup_pulses += 1
        local ok, err = nsmbu.input.press("A", 3)
        note(step, string.format("press A file_select phase=%s", phase_text()))
        if ok == nil and err ~= nil and not press_err_logged then
          press_err_logged = true
          note(step, err)
        end
      end
    else
      local ok, err = scene.open_title()
      if ok == true then
        note(step, "open_title")
      else
        note(step, string.format("open_title %s", err or "refused"))
      end
    end
    return
  end

  file_select_steps = 0
  popup_pulses = 0
  note(step, string.format("apply_save %s phase=%s", err or "refused", phase_text()))
end

function hooks.on_unload()
  done = false
  opened = false
  requested = false
  last = ""
  file_select_steps = 0
  popup_pulses = 0
  press_err_logged = false
end
