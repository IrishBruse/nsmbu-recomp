const scene = nsmbu.scene

local stage = 0
local hold = 0

local function settled(steps: number)
  hold += 1
  if hold < steps then
    return false
  end
  hold = 0
  return true
end

nsmbu.on_logic_step(function(step: number)
  if stage == 0 then
    if scene.title() then
      scene.open_file_select()
      stage = 1
      hold = 0
    end
  elseif stage == 1 then
    if not scene.title() and settled(45) then
      scene.confirm_file()
      stage = 2
    end
  elseif stage == 2 then
    if scene.course_select() then
      stage = 3
    elseif scene.course() or scene.map_phase() then
      if settled(30) then
        scene.open_course_select()
        stage = 3
      end
    end
  elseif stage == 3 then
    if scene.course_select() then
      stage = 4
      nsmbu.log(string.format("world map at step %d", step))
    end
  end
end)

nsmbu.on_unload(function()
  stage = 0
  hold = 0
end)
