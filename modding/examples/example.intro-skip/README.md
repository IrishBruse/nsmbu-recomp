# Intro skip

Loads to the world map without input.

Direct mode is the default.
It waits for the boot flow to settle, then calls `nsmbu.scene.open_course_select()`
once the map phase has been idle for a while.
If the flow still stalls, it calls `nsmbu.scene.confirm_file()` to finish the load.

Press mode drives the boot the old way.
It presses A across the title and save select, then stops once the world map
select task is alive.
The `slot` option navigates the save select: while the save select phase is seen,
it presses Down per slot and then A, so slot 2 or 3 still loads that save.

Options:

| Option | Meaning |
| --- | --- |
| Mode | `direct` opens the course select from the map phase (default), `press` drives the menus with injected presses. |
| Save slot | Which save the save-select plan picks (press mode). |
| Skip logic step | Earliest step direct mode opens the course select (default 120). |
| Steps between presses / Press hold steps | Pacing for injected presses (press mode). |
| Log progress | Prints phase changes and transitions to the game log. |

The slot navigation timing is tuned for the dev save layout; if a different
system loads the save select at a different speed, raise the press spacing.
