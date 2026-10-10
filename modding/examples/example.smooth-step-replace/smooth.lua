local smooth = {}

function smooth.step(value, target, scale, max_step)
  if value == target then
    return value
  end
  local delta = (target - value) * scale
  if delta > max_step then
    delta = max_step
  elseif delta < -max_step then
    delta = -max_step
  end
  return value + delta
end

return smooth
