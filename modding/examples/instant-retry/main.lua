local player = require("player")
local state = require("state")
local death = require("death")

state.load_file()
death.install()

local function record_safe(step)
  if not nsmbu.config.bool("enabled", true) then
    return
  end
  if state.mode ~= "idle" then
    return
  end
  local every = nsmbu.config.number("record_every", 10)
  if every < 1 then
    every = 1
  end
  if step % every ~= 0 then
    return
  end
  local actor, err = player.actor()
  if not actor then
    return
  end
  local pose, pose_err = player.read_pose(actor)
  if not pose then
    nsmbu.log(pose_err)
    return
  end
  if not pose.grounded then
    return
  end
  if pose.speed_y < -1 or pose.speed_y > 1 then
    return
  end
  state.safe = pose
  if step % 60 == 0 then
    state.set_status(
      string.format(
        "Safe %.0f,%.0f (retries %d)",
        pose.x,
        pose.y,
        state.retries
      )
    )
  end
end

local function on_course_enter()
  state.reset_course()
  state.set_status("Instant retry: waiting for ground")
end

local enter = nsmbu.fn.course_enter
if enter then
  local id, err = nsmbu.hook(enter, "return", function()
    on_course_enter()
  end)
  if not id then
    nsmbu.log(err)
  end
else
  nsmbu.log("course_enter is not in nsmbu.fn")
end

local checkpoint = nsmbu.fn.checkpoint_touched
if checkpoint then
  local id, err = nsmbu.hook(checkpoint, "entry", function(ctx)
    local actor = player.actor()
    if not actor then
      return
    end
    local pose = player.read_pose(actor)
    if pose then
      state.safe = pose
      state.set_status("Soft spot set from checkpoint")
    end
  end)
  if not id then
    nsmbu.log(err)
  end
else
  nsmbu.log("checkpoint_touched is not in nsmbu.fn")
end

function nsmbu.on_logic_step(step)
  death.tick_hold()
  record_safe(step)
end

function nsmbu.on_config_changed()
  if not nsmbu.config.bool("enabled", true) then
    state.mode = "idle"
    state.set_status("Instant retry off")
  end
end

function nsmbu.on_unload()
  state.save_file()
end
