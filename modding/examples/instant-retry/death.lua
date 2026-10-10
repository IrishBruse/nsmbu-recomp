local player = require("player")
local state = require("state")

local death = {}

local function spend_life()
  if not nsmbu.config.bool("cost_life", false) then
    return true
  end
  local lose = nsmbu.fn.player_lose_life
  if not lose then
    return nil, "player_lose_life is not in nsmbu.fn"
  end
  return nsmbu.call(lose, {})
end

local function quit_course()
  local quit = nsmbu.fn.course_quit_to_map
  if not quit then
    return nil, "course_quit_to_map is not in nsmbu.fn"
  end
  return nsmbu.call(quit, {})
end

function death.begin(ctx)
  if not nsmbu.config.bool("enabled", true) then
    return
  end
  if state.mode ~= "idle" then
    ctx.skip = true
    return
  end
  if not state.safe then
    state.set_status("Instant retry: no safe spot yet")
    return
  end
  ctx.skip = true
  state.mode = "restoring"
  state.quit_hold = 0
  local actor, err = player.actor()
  if not actor then
    state.mode = "idle"
    nsmbu.log(err)
    return
  end
  local ok, restore_err = player.restore(
    actor,
    state.safe,
    nsmbu.config.bool("keep_powerup", true)
  )
  if not ok then
    state.mode = "idle"
    nsmbu.log(restore_err)
    return
  end
  local spent, life_err = spend_life()
  if not spent then
    nsmbu.log(life_err)
  end
  state.retries = state.retries + 1
  state.mode = "holding"
  state.set_status(
    string.format("Retry #%d — hold jump to quit", state.retries)
  )
end

function death.tick_hold()
  if state.mode ~= "holding" then
    return
  end
  local need = nsmbu.config.number("quit_hold", 45)
  if nsmbu.button.held("A") then
    state.quit_hold = state.quit_hold + 1
  else
    state.quit_hold = 0
    state.mode = "idle"
    state.set_status(
      string.format("Instant retry ready (%d)", state.retries)
    )
    return
  end
  if state.quit_hold < need then
    state.set_status(
      string.format("Hold jump %d/%d to quit", state.quit_hold, need)
    )
    return
  end
  local ok, err = quit_course()
  if not ok then
    nsmbu.log(err)
    state.mode = "idle"
    return
  end
  state.reset_course()
  state.set_status("Quit to map")
end

function death.install()
  local target = nsmbu.fn.player_begin_death
  if not target then
    nsmbu.log("player_begin_death is not in nsmbu.fn")
    return
  end
  local id, err = nsmbu.hook(target, "entry", death.begin)
  if not id then
    nsmbu.log(err)
  end
end

return death
