const config = nsmbu.config
const scene = nsmbu.scene
const press = nsmbu.input.press

local enabled = false
local debug = false
local mode = "direct"
local slot = 1
local skip_at = 120
local confirm_wait = 30
local press_every = 20
local press_hold = 3

local next_press = 0
local next_confirm = 0
local stable_since = -1
local nav_queue: { { button: string, at: number } } = {}
local nav_planned = false
local opened = false
local confirmed = false
local stopped = false

local function read_config()
	enabled = config.bool("enabled", false) or false
	debug = config.bool("debug", false) or false
	mode = config.string("mode", "direct") or "direct"
	slot = config.number("slot", 1) or 1
	skip_at = config.number("skip_at", 120) or 120
	confirm_wait = config.number("confirm_wait", 30) or 30
	press_every = config.number("press_every", 20) or 20
	press_hold = config.number("press_hold", 3) or 3
	if slot < 1 or slot > 3 then
		slot = 1
	end
	if skip_at < 10 then
		skip_at = 10
	end
	if confirm_wait < 1 then
		confirm_wait = 1
	end
	if press_every < 2 then
		press_every = 2
	end
	if press_hold < 1 then
		press_hold = 1
	end
	if mode ~= "press" then
		mode = "direct"
	end
end

local function plan_file_nav(step: number)
	nav_planned = true
	local at = step + 30
	for _ = 2, slot do
		table.insert(nav_queue, { button = "Down", at = at })
		at += 45
	end
	table.insert(nav_queue, { button = "A", at = at })
	if debug then
		nsmbu.log(string.format("file select at %d, slot %d planned", step, slot))
	end
end

nsmbu.on_logic_step(function(step)
	if step % 15 == 0 then
		read_config()
	end
	if not enabled then
		return
	end
	if debug and step % 15 == 0 then
		local gates = "-"
		local scene_ptr = nsmbu.guest.read_u32(0x101E7B28)
		if scene_ptr and scene_ptr ~= 0 then
			local flow_ref = nsmbu.guest.read_u32(scene_ptr + 0x14)
			if flow_ref and flow_ref ~= 0 then
				local b1 = nsmbu.guest.read_u8(flow_ref - 4 + 0xDB1)
				local b2 = nsmbu.guest.read_u8(flow_ref - 4 + 0xDB2)
				gates = string.format("%d/%d", b1 or -1, b2 or -1)
			end
		end
		nsmbu.log(string.format("step %d phase %08X title=%s filesel=%s cs=%s course=%s gates=%s",
			step, scene.phase() or 0, tostring(scene.title()), tostring(scene.file_select()),
			tostring(scene.course_select()), tostring(scene.course()), gates))
	end
	if not stopped and scene.course_select() then
		stopped = true
		nsmbu.log(string.format("course select at step %d", step))
	end
	if stopped then
		return
	end

	if mode == "direct" then
		if opened then
			if not confirmed and step >= next_confirm then
				next_confirm = step + 30
				local ok, err = scene.confirm_file()
				if ok then
					confirmed = true
					nsmbu.log(string.format("confirm_file ok at step %d", step))
				else
					nsmbu.log("confirm_file failed: " .. tostring(err))
				end
			end
			return
		end
		local idle = scene.map_phase() and scene.course() and not scene.course_select()
		if idle then
			if stable_since < 0 then
				stable_since = step
			end
		else
			stable_since = -1
		end
		if stable_since >= 0 and step >= skip_at and step - stable_since >= 60 and step % 5 == 0 then
			local ok, err = scene.open_course_select()
			if ok then
				opened = true
				next_confirm = step + confirm_wait
				nsmbu.log(string.format("open_course_select ok at step %d", step))
			else
				nsmbu.log("open_course_select failed: " .. tostring(err))
			end
		end
		return
	end

	if not nav_planned and scene.file_select() then
		plan_file_nav(step)
	end
	if #nav_queue > 0 then
		local item = nav_queue[1]
		if step >= item.at then
			table.remove(nav_queue, 1)
			press(item.button, press_hold)
			if debug then
				nsmbu.log(string.format("nav press %s at %d", item.button, step))
			end
		end
		return
	end
	if step >= next_press then
		press("A", press_hold)
		if debug then
			nsmbu.log(string.format("press A at %d", step))
		end
		next_press = step + press_every
	end
end)

nsmbu.on_loaded(function()
	nsmbu.log("loaded")
end)

nsmbu.on_config_changed(read_config)
nsmbu.on_unload(function()
	next_press = 0
	nav_queue = {}
end)
read_config()
