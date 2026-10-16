const Level = nsmbu.Level
const Player = nsmbu.Player
const Config = nsmbu.Config
const Hooks = nsmbu.Hooks

local held_mode: { [number]: number? } = {}
local prev_speed_y: { [number]: number? } = {}
local scaled_jump: { [number]: boolean? } = {}

local function freeze_timer()
	Level.set_time(999)
end

local function keep_lives()
	for _, p in Player.get_players() do
		local lives = p:lives()
		if lives and lives > 0 and lives < 99 then
			p:set_lives(99)
		end
	end
end

local function keep_powerup()
	for i = 0, 3 do
		local p = Player.get_player(i)
		if not p or not p:entered() then
			held_mode[i] = nil
			continue
		end
		local mode = p:mode()
		if not mode then
			continue
		end
		local held = held_mode[i]
		if held and held >= mode then
			if mode < held and held > 0 then
				p:set_mode(held)
			end
		else
			held_mode[i] = mode
		end
	end
end

local function scale_jump(p: any, i: number, scale: number)
	local speed = p:speed_y()
	if not speed then
		prev_speed_y[i] = nil
		scaled_jump[i] = false
		return
	end
	local prev = prev_speed_y[i] or 0
	if speed > 1.5 and prev <= 1.5 then
		scaled_jump[i] = false
	elseif speed <= 0 then
		scaled_jump[i] = false
	elseif not scaled_jump[i] and speed > 1.5 then
		p:set_speed_y(speed * scale)
		scaled_jump[i] = true
	end
	prev_speed_y[i] = speed
end

local function scale_jumps(scale: number)
	for i = 0, 3 do
		local p = Player.get_player(i)
		if not p then
			continue
		end

		scale_jump(p, i, scale)
	end
end

function Hooks.onLoaded() end

function Hooks.onLogicStep(_step: number)
	if Config.bool("infinite_time", false) then
		freeze_timer()
	end
	if Config.bool("infinite_lives", false) then
		keep_lives()
	end
	if Config.bool("keep_powerup", false) then
		keep_powerup()
	end
	local scale = Config.number("jump_scale", 1)
	if scale and scale ~= 1 then
		scale_jumps(scale)
	end
end
