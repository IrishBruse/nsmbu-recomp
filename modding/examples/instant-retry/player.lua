local player = {}

local OFF = {
  x = 0x314,
  y = 0x318,
  z = 0x31C,
  speed_y = 0x340,
  grounded = 0x3A0,
  powerup = 0x440,
}

local function need(name)
  local addr = nsmbu.fn[name]
  if not addr then
    return nil, name .. " is not in nsmbu.fn"
  end
  return addr
end

function player.actor()
  local getter = need("player_actor_ptr")
  if not getter then
    return nil, "player_actor_ptr is not in nsmbu.fn"
  end
  local result, err = nsmbu.call_result(getter, {})
  if not result then
    return nil, err
  end
  if result.r3 == 0 then
    return nil, "no player actor"
  end
  return result.r3
end

function player.read_pose(actor)
  local x, err = nsmbu.guest.read_f32(actor + OFF.x)
  if not x then
    return nil, err
  end
  local y = nsmbu.guest.read_f32(actor + OFF.y)
  local z = nsmbu.guest.read_f32(actor + OFF.z)
  local speed_y = nsmbu.guest.read_f32(actor + OFF.speed_y)
  local grounded = nsmbu.guest.read_u8(actor + OFF.grounded)
  local powerup = nsmbu.guest.read_u8(actor + OFF.powerup)
  if not y or not z or not speed_y or not grounded or not powerup then
    return nil, "player pose read failed"
  end
  return {
    x = x,
    y = y,
    z = z,
    speed_y = speed_y,
    grounded = grounded ~= 0,
    powerup = powerup,
  }
end

function player.write_pose(actor, pose)
  local ok = nsmbu.guest.write_f32(actor + OFF.x, pose.x)
  ok = ok and nsmbu.guest.write_f32(actor + OFF.y, pose.y)
  ok = ok and nsmbu.guest.write_f32(actor + OFF.z, pose.z)
  ok = ok and nsmbu.guest.write_f32(actor + OFF.speed_y, 0)
  if not ok then
    return nil, "player pose write failed"
  end
  return true
end

function player.write_powerup(actor, powerup)
  return nsmbu.guest.write_u8(actor + OFF.powerup, powerup)
end

function player.restore(actor, pose, keep_powerup)
  local ok, err = player.write_pose(actor, pose)
  if not ok then
    return nil, err
  end
  if keep_powerup then
    local wrote, write_err = player.write_powerup(actor, pose.powerup)
    if not wrote then
      return nil, write_err
    end
  end
  local clear = need("player_clear_damage")
  if clear then
    local called, call_err = nsmbu.call(clear, { r3 = actor })
    if not called then
      return nil, call_err
    end
  end
  return true
end

return player
