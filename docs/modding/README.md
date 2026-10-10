# Modding

This folder is the modding documentation for NSMBU recomp.

## Live

| Doc | What it covers |
| --- | --- |
| [mod-manager.md](mod-manager.md) | In-game Mods tab, install, profiles, storage, shared manifest fields |
| [content.md](content.md) | `kind: content` file replacements (models, textures, UI, fan translations) |
| [cemu.md](cemu.md) | `kind: cemu` graphics and shader packs |

Rejected on install: `kind: native`, `kind: guest`, and `kind: settings`.

## Proposal (not implemented)

| Doc | What it covers |
| --- | --- |
| [lua-mods.md](lua-mods.md) | Plan to replace content/native/guest with Lua packages |
| [api.md](api.md) | Proposed Lua host API |

Example packages (also not loaded by the runtime): [../../examples/lua-mods/](../../examples/lua-mods/).

## Historical archives

| Doc | What it was |
| --- | --- |
| [../deprecated/native-sdk-v1.md](../deprecated/native-sdk-v1.md) | Host `dlopen` C ABI (`kind: native`) |
| [../deprecated/mod-sdk-v2.md](../deprecated/mod-sdk-v2.md) | PowerPC guest mods (`kind: guest`) |

Do not use the archives as modder guides.
