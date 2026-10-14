# Modding

This folder is the modding documentation for NSMBU recomp.

## Live

| Doc | What it covers |
| --- | --- |
| [mod-manager.md](mod-manager.md) | In-game Mods tab, install, profiles, storage, shared manifest fields |
| [luau-mods.md](luau-mods.md) | Luau packages (phase 2 scripting; phase 3 `content_dir`) |
| [api.md](api.md) | Luau host API (phase 2 live; later phases still proposal) |
| [content.md](content.md) | File replacements via Luau `content_dir` (`kind: content` is a legacy alias this release) |
| [cemu-content.md](cemu-content.md) | Drop-in Cemu level/content packs: Definition-only `rules.txt` + `content/`, ignore `meta/` |
| [cemu.md](cemu.md) | `kind: cemu` graphics and shader packs |

Rejected on install: `kind: native`, `kind: guest`, and `kind: settings`.
`kind: content` is accepted as a read-only alias this release and migrates to `luau` on scan.

## Proposal (not implemented)

| Doc | What it covers |
| --- | --- |
| [luau-mods.md](luau-mods.md) phases 4–5 | Hooks, `nsmbu.hooks.call`, and hard rejection of the content alias |
| [api.md](api.md) (hooks, calls, input, data, guest heap) | Host APIs still marked proposal in that doc |

Example packages: [../../modding/examples/](../../modding/examples/).
Luau SDK: [../../modding/sdk/](../../modding/sdk/).

## Investigation

| Doc | What it covers |
| --- | --- |
| [intro-skip.md](intro-skip.md) | Stable boot path for `example.intro-skip` |
