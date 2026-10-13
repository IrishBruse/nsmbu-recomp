const scene = nsmbu.scene
const press = nsmbu.input.press

local next_press = 0

nsmbu.on_logic_step(function(step)
	if step % 15 == 0 then
		nsmbu.log(string.format("step %d phase %08X cs=%s course=%s", step,
			scene.phase() or 0, tostring(scene.course_select()), tostring(scene.course())))
	end
	if scene.course_select() then
		return
	end
	if step >= next_press then
		press("A", 3)
		next_press = step + 60
	end
end)
