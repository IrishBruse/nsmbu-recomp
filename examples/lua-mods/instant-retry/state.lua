local state = {
  mode = "idle",
  safe = nil,
  retries = 0,
  quit_hold = 0,
  last_status = "",
}

function state.reset_course()
  state.mode = "idle"
  state.safe = nil
  state.retries = 0
  state.quit_hold = 0
  state.last_status = ""
end

function state.set_status(text)
  if text == state.last_status then
    return
  end
  state.last_status = text
  nsmbu.status(text)
end

function state.save_file()
  if not state.safe then
    nsmbu.data.remove("softspot.txt")
    return
  end
  local text = string.format(
    "%.9g %.9g %.9g %.9g %d %d\n",
    state.safe.x,
    state.safe.y,
    state.safe.z,
    state.safe.speed_y,
    state.safe.powerup,
    state.retries
  )
  local wrote, err = nsmbu.data.write("softspot.txt", text)
  if not wrote then
    nsmbu.log(err)
  end
end

function state.load_file()
  local text = nsmbu.data.read("softspot.txt")
  if not text then
    return
  end
  local x, y, z, speed_y, powerup, retries = text:match(
    "^(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%d+)%s+(%d+)"
  )
  if not x then
    return
  end
  state.safe = {
    x = tonumber(x),
    y = tonumber(y),
    z = tonumber(z),
    speed_y = tonumber(speed_y),
    grounded = true,
    powerup = tonumber(powerup),
  }
  state.retries = tonumber(retries) or 0
end

return state
