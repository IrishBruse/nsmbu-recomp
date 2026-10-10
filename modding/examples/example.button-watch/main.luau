local nsmbu = require("nsmbu")
local config = require("nsmbu.config")
local input = require("nsmbu.input")

local last: string = ""

local function every_n(): number
  local every = config.number("every", 15)
  if every < 1 then
    return 1
  end
  return every
end

function nsmbu.on_logic_step(step: number)
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
    config.string("label", "Pad"),
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
