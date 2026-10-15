# Intro skip

The skip stops on the first save slot.
It does not open the time-attack page.

`hooks.on_logic_step` drives the skip while the boot flow runs.
While `title()` or `map_phase()` is true, the script calls `open_file_select()`.
After the first attempt it keeps calling until the call returns true.
When that call returns true, the script logs the save slot page.
It then reads `file_index()`.
When the file number is 0, the script logs save slot 0 and stops.

The script sends `open_file_select()` again when the call returns `"scene busy"`, `"menu missing"`, `"state not ready"`, or `"file not ready"`.
It does not call `confirm_file()` or `open_course_select()`.
`save_select()` is a probe for `ChallengeSelectScene` (time attack).
The script logs and waits if that probe becomes true.
The script does not treat that probe as success.

No buttons are pressed and no guest memory is read.

[docs/modding/intro-skip.md](../../../docs/modding/intro-skip.md) records the boot tasks behind these probes.

Options:

| Option | Meaning |
| --- | --- |
| Enabled | Turn the skip on or off. |
| Log progress | Print each successful transition to the game log. |
