# Intro skip

Loads to the world map without input.

`nsmbu.on_loaded` starts the skip once the package is live.
It then calls three scene transitions in order:

1. `nsmbu.scene.open_file_select()`, the call pressing A on the title makes.
2. `nsmbu.scene.confirm_file()`, the call pressing A on the save select makes.
3. `nsmbu.scene.open_course_select()`, the call that opens the world map.

No buttons are pressed and no guest memory is read.
The scene calls already do the work the button presses would do.

Options:

| Option | Meaning |
| --- | --- |
| Enabled | Turn the skip on or off. |
| Log progress | Print each successful transition to the game log. |