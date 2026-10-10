local fn = require("nsmbu.fn")
local hook = require("nsmbu.hook")
local api = require("nsmbu.player")

local player = {}

local function need(name)
  local addr = fn[name]
  if not addr then
    return nil, name .. " is not in nsmbu.fn"
  end
  return addr
end

function player.handle()
  local p, err = api(0)
  if not p then
    return nil, err or "no player"
  end
  return p
end

function player.actor()
  local p, err = player.handle()
  if not p then
    return nil, err
  end
  return p:actor()
end

function player.read_pose()
  local p, err = player.handle()
  if not p then
    return nil, err
  end
  local pos, pos_err = p:pos()
  if not pos then
    return nil, pos_err
  end
  local speed_y, speed_err = p:speed_y()
  if speed_y == nil then
    return nil, speed_err
  end
  local grounded, ground_err = p:grounded()
  if grounded == nil then
    return nil, ground_err
  end
  local mode, mode_err = p:mode()
  if mode == nil then
    return nil, mode_err
  end
  return {
    x = pos.x,
    y = pos.y,
    z = pos.z,
    speed_y = speed_y,
    grounded = grounded,
    powerup = mode,
  }
end

function player.write_pose(pose)
  local p, err = player.handle()
  if not p then
    return nil, err
  end
  local ok, set_err = p:set_pos(pose.x, pose.y, pose.z)
  if not ok then
    return nil, set_err
  end
  ok, set_err = p:set_speed_y(0)
  if not ok then
    return nil, set_err
  end
  return true
end

function player.restore(pose, keep_powerup)
  local ok, err = player.write_pose(pose)
  if not ok then
    return nil, err
  end
  if keep_powerup then
    local p, handle_err = player.handle()
    if not p then
      return nil, handle_err
    end
    local wrote, write_err = p:set_mode(pose.powerup)
    if not wrote then
      return nil, write_err
    end
  end
  local actor, actor_err = player.actor()
  if not actor then
    return nil, actor_err
  end
  local clear = need("player_clear_damage")
  if clear then
    local called, call_err = hook.call(clear, { r3 = actor })
    if not called then
      return nil, call_err
    end
  end
  return true
end

return player
