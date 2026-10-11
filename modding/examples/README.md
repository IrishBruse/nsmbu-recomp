# Example Luau mods

These packages follow [docs/modding/api.md](../../docs/modding/api.md).
Phase 2 loads `kind: luau` with logic-step, config, log, status, and guest memory (embedded Luau).
The runtime exposes one global, `nsmbu`, with the helpers and one field per module.
Hooks, `nsmbu.fn`, input, and data files are still proposal.
They contain no game files.

Luau type definitions for the `nsmbu` API live in [../sdk/](../sdk/).
`nsmbu.d.luau` is the entry and declares the global; the other `.d.luau` files hold the types, one file per module.
Register all of them in load order (core, guest, hooks, input, data, level, player, then `nsmbu.d.luau` last), for example:

```json
"luau-lsp.types.definitionFiles": {
    "nsmbu-core": "modding/sdk/core.d.luau",
    "nsmbu-guest": "modding/sdk/guest.d.luau",
    "nsmbu-hooks": "modding/sdk/hooks.d.luau",
    "nsmbu-input": "modding/sdk/input.d.luau",
    "nsmbu-data": "modding/sdk/data.d.luau",
    "nsmbu-level": "modding/sdk/level.d.luau",
    "nsmbu-player": "modding/sdk/player.d.luau",
    "nsmbu": "modding/sdk/nsmbu.d.luau"
}
```

The definitions are editor-only; the runtime builds the same table.

| Mod | What it shows | Phase |
| --- | --- | --- |
| `example.play-scene-ticker` | Entry and return hooks, a number option, and a data file written on unload. | Proposal (hooks, data) |
| `example.smooth-step-replace` | A replacement that calls `hooks.original` on odd calls and edits the guest float on even calls. | Proposal (hooks) |
| `example.button-watch` | A `logic_step` listener, pad input, and the Mods status line. | Partial (logic-step and status work; input is proposal) |
| `example.cheats` | Course timer, lives, power-up, and jump cheats through high-level APIs. | Phase 2 |
| `example.instant-retry` | Soft checkpoint retry with state, restore calls, and hold-jump to quit. Inspired by Tsuru instant respawn. | Proposal (hooks, call) |

`example.play-scene-ticker` and `example.smooth-step-replace` use `nsmbu.fn.dScnPly_Execute` and `nsmbu.fn.cLib_addCalc2`.
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
