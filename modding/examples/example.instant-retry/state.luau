type Pose = {
	x: number,
	y: number,
	z: number,
	speed_y: number,
	grounded: boolean,
	powerup: number,
}

type State = {
	mode: string,
	safe: Pose?,
	retries: number,
	quit_hold: number,
	last_status: string,
	reset_course: () -> (),
	set_status: (text: string) -> (),
	save_file: () -> (),
	load_file: () -> (),
}

const log = nsmbu.log
const status = nsmbu.status
const data = nsmbu.data

local state = {} :: State
state.mode = "idle"
state.safe = nil
state.retries = 0
state.quit_hold = 0
state.last_status = ""

function state.reset_course()
  state.mode = "idle"
  state.safe = nil
  state.retries = 0
  state.quit_hold = 0
  state.last_status = ""
end

function state.set_status(text: string)
  if text == state.last_status then
    return
  end
  state.last_status = text
  status(text)
end

function state.save_file()
  if not state.safe then
    if data.remove then
      data.remove("softspot.txt")
    end
    return
  end
  if not data.write then
    log("nsmbu.data.write is not in data")
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
  local wrote, err = data.write("softspot.txt", text)
  if not wrote then
    log(err or "data write failed")
  end
end

function state.load_file()
  if not data.read then
    log("nsmbu.data.read is not in data")
    return
  end
  local text = data.read("softspot.txt")
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
    x = tonumber(x) or 0,
    y = tonumber(y) or 0,
    z = tonumber(z) or 0,
    speed_y = tonumber(speed_y) or 0,
    grounded = true,
    powerup = tonumber(powerup) or 0,
  }
  state.retries = tonumber(retries) or 0
end

return state
