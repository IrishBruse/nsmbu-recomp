# Instant retry

This package is a hypothetical gameplay mod, not a skip-one-instruction cheat.
The loader does not run it.
It contains no guest addresses from a real patch.

## Real feature it follows

[Tsuru](https://github.com/Zenith-Team/Tsuru) ships instant respawn for NSMBU v1.3.0.
On death the player returns inside the course.
Holding jump cancels that retry and leaves the course.
Checkpoints still matter for the soft spot.
This example copies that player-facing idea.
It does not copy Tsuru source.

## What the script does

1. While the player is grounded and almost still, sample position and power-up into a soft spot.
2. Touching a checkpoint also refreshes that soft spot.
3. When death starts, skip the normal death entry, restore the soft spot, optionally spend a life, and enter a short hold window.
4. Hold jump through `quit_hold` logic steps to call a map-quit helper.
5. On unload, write `softspot.txt` under the package data folder.

The work is split across files.

| File | Role |
| --- | --- |
| `main.luau` | Hooks course enter and checkpoint touch, records safe spots each logic step. |
| `death.luau` | Death entry hook, restore path, hold-to-quit. |
| `player.luau` | High-level `require("nsmbu.player")` pose helpers, plus `player_clear_damage` when bound. |
| `state.luau` | Mode machine, status line, data file. |

Pose reads use `pos()`, `speed_y()`, `grounded()`, and `mode()`.
Restore uses `set_pos`, `set_speed_y`, and `set_mode`.
No raw actor field offsets live in this package.

## Symbols that are not bound yet

`nsmbu.fn` only has a few names in this tree today.
The script names the helpers a finished API would need:

| Name | Use |
| --- | --- |
| `player_begin_death` | Entry hook. Set `ctx.skip` to take over. |
| `player_clear_damage` | Clears death and hit state after a restore. |
| `player_lose_life` | Optional life cost. |
| `course_enter` | Resets soft state when a course starts. |
| `checkpoint_touched` | Marks the soft spot from a real checkpoint. |
| `course_quit_to_map` | Leaves the course when the quit hold finishes. |

Actor lookup uses `p:actor()` from the high-level player API.
Until the death and map helpers exist in `nsmbu.fn`, the script logs missing symbols and leaves the game alone.

## Why this is not a simple patch

A Cemu timer or lives patch writes one value every frame.
This mod keeps a mode (`idle`, `restoring`, `holding`), samples physics, calls game helpers, and mixes pad input with restore logic.
That is the same class of change as Tsuru's instant respawn, expressed with the proposed Luau API.
