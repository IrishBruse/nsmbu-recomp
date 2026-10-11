const config = nsmbu.config
const input = nsmbu.input

local last: string = ""

local function every_n(): number
  local every = config.number("every", 15)
  if not every or every < 1 then
    return 1
  end
  return every
end

nsmbu.on_logic_step(function(step: number)
  if step % every_n() ~= 0 then
    return
  end
  local pad = input()
  local held = input.button.held("A")
  if held == nil then
    held = false
  end
  local line = string.format(
    "%s A=%s lx=%.2f",
    config.string("label", "Pad") or "Pad",
    tostring(held),
    pad.lx
  )
  if line == last then
    return
  end
  last = line
  nsmbu.status(line)
end)

nsmbu.on_config_changed(function()
  last = ""
end)
