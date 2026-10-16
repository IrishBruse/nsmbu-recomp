# Intro skip

The skip reads the Save file option.
That option is 1, 2, or 3.
The default is 1.
The skip calls `open_first_save(file)` once the title or the map phase is current.
`file` is the option minus 1.
That call writes the selected save file and runs guest `0x029872BC`.
The world map comes up when that file has a record.
An empty file follows the game branch to factory `0x0202B570`.

`hooks.on_logic_step` drives the skip while the boot flow runs.

1. After the title or the map phase is current, read the Save file option and call `open_first_save(file)`.
2. Stop when `course_select()` is true (world map).

Forcing menu factory `0x0202B61C` early shows the time-attack trial layout.
This example never requests that factory.
`save_select()` is a probe for `ChallengeSelectScene`.
The script logs and waits if that probe becomes true.

[docs/modding/intro-skip.md](../../../docs/modding/intro-skip.md) records the boot tasks behind these probes.
