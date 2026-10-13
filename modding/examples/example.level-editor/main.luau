const level = nsmbu.level
const player = nsmbu.player
const config = nsmbu.config

local PROFILE_IDS: { [string]: number } = {
	["coin"] = 395,
	["goomba"] = 467,
	["mushroom"] = 591,
	["fire flower"] = 592,
	["one-up"] = 597,
	["star"] = 853,
}

local spawned = 0
local spawned_failed = 0

local function spawn_id(): number?
	local name = config.string("spawn_kind", "off")
	if not name or name == "off" then
		return nil
	end
	return PROFILE_IDS[name]
end

local function spawn_countdown(step: number): boolean
	local every = config.number("spawn_every", 2) or 1
	return step % math.floor(math.max(every, 1) * 30) == 0
end

local function spawn_near_player()
	local id = spawn_id()
	if not id then
		return
	end
	local p = player.get_player(0)
	if not p or not p:entered() then
		return
	end
	local pos = p:pos()
	if not pos then
		return
	end
	local ox = config.number("spawn_offset_x", 0) or 0
	local oy = config.number("spawn_offset_y", 0) or 0
	local done, err = level.spawn(id, pos.x + ox, pos.y + oy + 32, pos.z)
	if done then
		spawned = spawned + 1
	else
		spawned_failed = spawned_failed + 1
		if spawned_failed == 1 then
			nsmbu.log(err or "spawn failed")
		end
	end
end

local function clear_kind_actors()
	local id = spawn_id()
	if not id then
		return
	end
	for _, a in level.actors() do
		local pid = a:profile_id()
		if pid ~= id then
			continue
		end
		a:delete()
	end
end

nsmbu.on_logic_step(function(step: number)
	if not level.active() then
		return
	end
	if config.bool("clear_kind", false) then
		clear_kind_actors()
	end
	if spawn_countdown(step) then
		spawn_near_player()
		nsmbu.status(("spawned %d, live %d"):format(spawned, level.count() or 0))
	end
end)

nsmbu.on_unload(function()
	nsmbu.log_int("spawned", spawned)
end)
