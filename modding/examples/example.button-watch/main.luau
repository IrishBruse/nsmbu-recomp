local last: string = ""

local function every_n(): number
  local every = nsmbu.config.number("every", 15)
  if not every or every < 1 then
    return 1
  end
  return every
end

nsmbu.listen("logic_step", function(step: number)
  if step % every_n() ~= 0 then
    return
  end
  local pad = nsmbu.input()
  local held = nsmbu.input.button.held("A")
  if held == nil then
    held = false
  end
  local line = string.format(
    "%s A=%s lx=%.2f",
    nsmbu.config.string("label", "Pad") or "Pad",
    tostring(held),
    pad.lx
  )
  if line == last then
    return
  end
  last = line
  nsmbu.status(line)
end)

nsmbu.listen("config_changed", function()
  last = ""
end)
