# Intro skip

The skip reads the Save file option.
That option is 1, 2, or 3.
The default is 1.
It reads the Course pack option.
The default is `1-1`.
A non-empty pack enters that stage with `scene.load_pack` after the save opens.
While the boot GamePad setup popup is up (`file_select()`), it presses **A** about every 30 logic steps.
After four presses, if the popup is still up, it calls `open_title()` until the title or map phase is current.
Then it calls `open_first_save(file)` with `file` equal to the option minus 1.
That call writes the selected save file and runs guest `0x029872BC`.
The world map comes up when that file has a record.
An empty file follows the game branch to factory `0x0202B570`.

`hooks.on_logic_step` drives the skip while the boot flow runs.

1. While `file_select()` is true, pulse **A** (up to four times), then retry `open_title()` if needed.
2. After the title or the map phase is current, read the Save file option and call `open_first_save(file)`.
3. Call `scene.load_pack` for the Course pack. The default pack is `1-1`.
4. Stop when `course_task()` is true.

Forcing menu factory `0x0202B61C` early shows the time-attack trial layout.
This example never requests that factory.
`save_select()` is a probe for `ChallengeSelectScene`.
The script logs and waits if that probe becomes true.

[docs/modding/intro-skip.md](../../../docs/modding/intro-skip.md) records the boot tasks behind these probes.
That doc also covers open work on which confirm button the popup layout uses.
