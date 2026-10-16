# Intro skip

The skip reads the Save file option.
That option is 1, 2, or 3.
The default is 1.
It reads the Course pack option.
The default is `1-1`.
A non-empty pack enters that stage with `scene.load_pack` after the save opens.
While the boot GamePad setup popup is up (`file_select()`), it presses **A** about every 30 logic steps.
After four presses, if the popup is still up, it calls `open_title()` until the title or map phase is current.
It calls `apply_save(file)` after the boot logo.
Applying the save during the logo sends the flow into course select before that task has a course node, and course select then crashes.
That call does not open the world map.
`file` is the option minus 1.

`hooks.on_logic_step` drives the skip while the boot flow runs.

1. While `file_select()` is true, pulse **A** (up to four times), then retry `open_title()` if needed.
2. After the boot logo, call `apply_save` until the save is applied.
3. Call `scene.load_pack` once. The default pack is `1-1`.
4. Stop when `course_task()` is true.

Forcing menu factory `0x0202B61C` early shows the time-attack trial layout.
This example never requests that factory.
`save_select()` is a probe for `ChallengeSelectScene`.
The script logs and waits if that probe becomes true.

[docs/modding/intro-skip.md](../../../docs/modding/intro-skip.md) records the boot tasks behind these probes.
That doc also covers open work on which confirm button the popup layout uses.
