const Scene = nsmbu.Scene
const Hooks = nsmbu.Hooks
const Config = nsmbu.Config
const Input = nsmbu.Input
const Factory = Scene.Factory

local done = false
local opened = false
local requested = false
local last = ""
local logic_step = 0
local pending_popup = false
local pending_apply = false
local pending_load = false
local pending_course_up = false
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
	local phase = Scene.phase()
	if phase == nil then
		return "none"
	end
	return string.format("%08X", phase)
end

local function save_index(): number
	local picked = Config.string("save_file", "1")
	if picked == "2" then
		return 1
	elseif picked == "3" then
		return 2
	end
	return 0
end

local function course_pack(): string
	local pack = Config.string("course_pack", "1-1") or "1-1"
	if pack == "" then
		return "1-1"
	end
	return pack
end

local function try_load_pack(step: number)
	local pack = course_pack()
	local ok, err = Scene.load_pack(pack)
	if ok == true then
		requested = true
		pending_load = false
		note(step, string.format("load_pack %s", pack))
		return
	end
	if err == "scene busy" or (err ~= nil and string.find(err, "scene busy", 1, true) ~= nil) then
		pending_load = true
	else
		pending_load = true
		note(step, string.format("load_pack %s %s", pack, err or "refused"))
	end
end

local function try_apply_save(step: number)
	local save = save_index()
	local ok, err = Scene.apply_save(save)
	if ok == true then
		opened = true
		pending_apply = false
		pending_popup = false
		pending_load = true
		file_select_steps = 0
		popup_pulses = 0
		note(step, string.format("apply save %d", save + 1))
		return
	end
	if
		err == "file not ready"
		or err == "scene busy"
		or (err ~= nil and string.find(err, "scene busy", 1, true) ~= nil)
	then
		pending_apply = true
	else
		pending_apply = true
		note(step, string.format("apply_save %s phase=%s", err or "refused", phase_text()))
	end
end

function Hooks.onSceneChange(factory: number)
	if done then
		return
	end
	if factory == Factory.SaveSelect then
		note(logic_step, "unexpected challenge select")
		done = true
		return
	end
	if factory == Factory.CourseTask then
		if requested then
			pending_course_up = true
		end
		return
	end
	if factory == Factory.FileSelect then
		pending_popup = true
		pending_apply = true
		file_select_steps = 0
		popup_pulses = 0
		return
	end
	if factory == Factory.Boot then
		note(logic_step, "wait boot")
		return
	end
	if not opened then
		pending_apply = true
		return
	end
	if not requested then
		pending_load = true
	end
end

function Hooks.onLogicStep(step: number)
	logic_step = step
	if done then
		return
	end
	if pending_course_up or (requested and Scene.course_task()) then
		note(step, "course up")
		done = true
		return
	end
	if not pending_popup and not pending_apply and not pending_load then
		return
	end
	if pending_popup then
		if not Scene.file_select() then
			pending_popup = false
			file_select_steps = 0
			popup_pulses = 0
		else
			file_select_steps += 1
			if popup_pulses < 4 then
				if file_select_steps % 30 == 0 then
					popup_pulses += 1
					local ok, err = Input.press("A", 3)
					note(step, string.format("press A file_select phase=%s", phase_text()))
					if ok == nil and err ~= nil and not press_err_logged then
						press_err_logged = true
						note(step, err)
					end
				end
			else
				local ok, err = Scene.open_title()
				if ok == true then
					note(step, "open_title")
				else
					note(step, string.format("open_title %s", err or "refused"))
				end
			end
		end
	end
	if pending_apply and not opened then
		try_apply_save(step)
	end
	if pending_load and opened and not requested then
		try_load_pack(step)
	end
end

function Hooks.onUnload()
	done = false
	opened = false
	requested = false
	last = ""
	logic_step = 0
	pending_popup = false
	pending_apply = false
	pending_load = false
	pending_course_up = false
	file_select_steps = 0
	popup_pulses = 0
	press_err_logged = false
end
