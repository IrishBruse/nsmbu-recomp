local last = ""

local function every_n()
  local every = nsmbu.config.number("every", 15)
  if every < 1 then
    return 1
  end
  return every
end

function nsmbu.on_logic_step(step)
  if step % every_n() ~= 0 then
    return
  end
  local pad = nsmbu.input()
  local held = nsmbu.button.held("A")
  if held == nil then
    held = false
  end
  local line = string.format(
    "%s A=%s lx=%.2f",
    nsmbu.config.string("label", "Pad"),
    tostring(held),
    pad.lx
  )
  if line == last then
    return
  end
  last = line
  nsmbu.status(line)
end

function nsmbu.on_config_changed()
  last = ""
end
