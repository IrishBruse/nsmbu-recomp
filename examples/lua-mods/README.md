# Example Lua mods (proposal)

These packages follow [docs/modding/api.md](../../docs/modding/api.md).
The loader does not run them yet.
They contain no game files.

`sdk/nsmbu.d.lua` is the EmmyLua type file for the proposed global `nsmbu` API.
`.luarc.json` in this folder points the Lua language server at that file.

| Mod | What it shows |
| --- | --- |
| `play-scene-ticker` | Entry and return hooks, a number option, and a data file written on unload. |
| `smooth-step-replace` | A replacement that calls `nsmbu.original` on odd calls and edits the guest float on even calls. |
| `button-watch` | `on_logic_step`, pad input, and the Mods status line. |
| `course-assists` | Simple course cheats (skip hooks). Kept as a contrast. |
| `instant-retry` | Soft checkpoint retry with state, restore calls, and hold-jump to quit. Inspired by Tsuru instant respawn. |

`play-scene-ticker` and `smooth-step-replace` use `nsmbu.fn.dScnPly_Execute` and `nsmbu.fn.cLib_addCalc2`.
Those names are the two entries in `runtime/guest/include/nsmbu/functions.h`.
`instant-retry` needs more generated names; see that package README.

Each folder is a package.
It has `manifest.json` and `main.lua`.
`smooth-step-replace` and `instant-retry` also load extra `.lua` files with `require`.
