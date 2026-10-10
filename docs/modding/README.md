# Modding

This folder is the modding documentation for NSMBU recomp.

## Live

| Doc | What it covers |
| --- | --- |
| [mod-manager.md](mod-manager.md) | In-game Mods tab, install, profiles, storage, shared manifest fields |
| [lua-mods.md](lua-mods.md) | Lua packages (phase 2 scripting; phase 3 `content_dir`) |
| [api.md](api.md) | Lua host API (phase 2 live; later phases still proposal) |
| [content.md](content.md) | File replacements via Lua `content_dir` (`kind: content` is a legacy alias this release) |
| [cemu.md](cemu.md) | `kind: cemu` graphics and shader packs |

Rejected on install: `kind: native`, `kind: guest`, and `kind: settings`.
`kind: content` is accepted as a read-only alias this release and migrates to `lua` on scan.

## Proposal (not implemented)

| Doc | What it covers |
| --- | --- |
| [lua-mods.md](lua-mods.md) phases 4–5 | Hooks, `nsmbu.call`, and hard rejection of the content alias |
| [api.md](api.md) (hooks, calls, input, data, guest heap) | Host APIs still marked proposal in that doc |

Example packages: [../../modding/examples/](../../modding/examples/).
EmmyLua SDK: [../../modding/sdk/](../../modding/sdk/).

## Historical archives

| Doc | What it was |
| --- | --- |
| [../deprecated/native-sdk-v1.md](../deprecated/native-sdk-v1.md) | Host `dlopen` C ABI (`kind: native`) |
| [../deprecated/mod-sdk-v2.md](../deprecated/mod-sdk-v2.md) | PowerPC guest mods (`kind: guest`) |

Do not use the archives as modder guides.
