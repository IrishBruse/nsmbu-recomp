# Intro skip

The skip presses A through the title path until the world map is up.
It does not open the menu task or the time-attack page.

`hooks.on_logic_step` drives the skip while the boot flow runs.

1. After the title path is current, press A on a timer.
2. Stop when `course_select()` is true (world map).

Forcing menu factory `0x0202B61C` early shows the time-attack trial layout.
This example never requests that factory.
`save_select()` is a probe for `ChallengeSelectScene`.
The script logs and waits if that probe becomes true.

[docs/modding/intro-skip.md](../../../docs/modding/intro-skip.md) records the boot tasks behind these probes.
