# Example Lua mods

These packages follow [docs/modding/api.md](../../docs/modding/api.md).
Phase 2 loads `kind: lua` with logic-step, config, log, status, and guest memory (embedded LuaJIT).
Hooks, `nsmbu.fn`, input, and data files are still proposal.
They contain no game files.

`sdk/nsmbu.d.lua` is the EmmyLua type file for the global `nsmbu` API.
`.luarc.json` in this folder points the Lua language server at that file.

| Mod | What it shows | Phase |
| --- | --- | --- |
| `play-scene-ticker` | Entry and return hooks, a number option, and a data file written on unload. | Proposal (hooks, data) |
| `smooth-step-replace` | A replacement that calls `nsmbu.original` on odd calls and edits the guest float on even calls. | Proposal (hooks) |
| `button-watch` | `on_logic_step`, pad input, and the Mods status line. | Partial (logic-step and status work; input is proposal) |
| `course-assists` | Simple course cheats (skip hooks). Kept as a contrast. | Proposal (hooks) |
| `instant-retry` | Soft checkpoint retry with state, restore calls, and hold-jump to quit. Inspired by Tsuru instant respawn. | Proposal (hooks, call) |

`play-scene-ticker` and `smooth-step-replace` use `nsmbu.fn.dScnPly_Execute` and `nsmbu.fn.cLib_addCalc2`.
Those names are the two entries in `runtime/guest/include/nsmbu/functions.h`.
`instant-retry` needs more generated names; see that package README.

Each folder is a package.
It has `manifest.json` and `main.lua`.
`smooth-step-replace` and `instant-retry` also load extra `.lua` files with `require`.
