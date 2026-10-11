# Example Luau mods

These packages follow [docs/modding/api.md](../../docs/modding/api.md).
Phase 2 loads `kind: luau` with logic-step, config, log, status, and guest memory (embedded Luau).
There is no `nsmbu` global; scripts `require("nsmbu")` and related modules.
Hooks, `nsmbu.fn`, input, and data files are still proposal.
They contain no game files.

Luau type definitions live in [../sdk/](../sdk/), one `.luau` module per `require` name.
`.luaurc` in this folder maps every `nsmbu.*` name to that folder for Luau tooling and sets the language mode.
For typing inside a mod, `require("nsmbu")` is a host module that the editor resolves as `any`.
The `.luaurc` aliases are for editor tooling only; scripts use the plain `require("nsmbu")` form, which is what the runtime resolves.

| Mod | What it shows | Phase |
| --- | --- | --- |
| `example.play-scene-ticker` | Entry and return hooks, a number option, and a data file written on unload. | Proposal (hooks, data) |
| `example.smooth-step-replace` | A replacement that calls `hooks.original` on odd calls and edits the guest float on even calls. | Proposal (hooks) |
| `example.button-watch` | `on_logic_step`, pad input, and the Mods status line. | Partial (logic-step and status work; input is proposal) |
| `example.course-assists` | Course timer, lives, power-up, and jump assists through high-level APIs. | Phase 2 |
| `example.instant-retry` | Soft checkpoint retry with state, restore calls, and hold-jump to quit. Inspired by Tsuru instant respawn. | Proposal (hooks, call) |

`example.play-scene-ticker` and `example.smooth-step-replace` use `fn.dScnPly_Execute` and `fn.cLib_addCalc2` from `require("nsmbu.fn")`.
Those names are the two entries in `runtime/guest/include/nsmbu/functions.h`.
`example.instant-retry` needs more generated names; see that package README.

Each folder is a package.
Its folder name is the package id.
It has `manifest.json` and `main.luau`.
`example.smooth-step-replace` and `example.instant-retry` also load extra `.luau` files with `require`.

`just launch` and `just run` link `user/ModManager/Mods` to this folder before start.
Edit here, then relaunch.
Enable a package in Mods (`` ` `` or F12).
Set `NSMBU_NO_EXAMPLE_MODS=1` to skip the link.
`NSMBU_MOD_MANAGER_DIR` also skips it (isolated test storage).
If `user/ModManager/Mods` is already a real folder with other packages, the examples are copied in instead.
