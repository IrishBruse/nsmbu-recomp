# Intro skip

Loads to the world map without input.

`hooks.on_logic_step` drives the skip while the boot flow runs.
It calls three scene transitions in order.
It sends each call again on the next logic step when the call returns `"scene busy"`.
It advances when the next probe is true.

1. While `title()` or `map_phase()` is true, `open_file_select()`.
   Advance when that call returns true.
2. While `save_select()` is true and `file_index()` returns a file, `confirm_file()`.
   Advance when that call returns true.
3. While `course()` and `course_task()` are true, `open_course_select()`.
   Stop when `course_select()` is true.

No buttons are pressed and no guest memory is read.
The scene calls already do the work the button presses would do.

[docs/modding/intro-skip.md](../../../docs/modding/intro-skip.md) records the boot tasks behind these probes.

Options:

| Option | Meaning |
| --- | --- |
| Enabled | Turn the skip on or off. |
| Log progress | Print each successful transition to the game log. |
