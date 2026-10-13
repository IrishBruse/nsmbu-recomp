## Local userdata (`nsmbu/`)

Gitignored folder used by `just` via `NSMBU_USER_DIR`.
Portable releases use the same layout under `data/nsmbu/`.

- `nsmbu/settings.ini` — overlay settings (Graphics, Display, Audio, and related keys)
- `nsmbu/controls.json` — controller bindings
- `nsmbu/save/` — Wii U save (`user/`, `common/`); passed as `--save nsmbu/save`
- `nsmbu/states/` — save-state slots and crash recovery
- `nsmbu/logs/` — run log (`nsmbu.log`), previous log, crash logs, frame dumps
- `nsmbu/shadercache/` — Vulkan pipeline and shader disk cache
- `nsmbu/ModManager/` — mod profiles and installed packages (`Mods/`)

## References (`references/`)

Upstream checkouts for names, decomp, emulator behaviour, and Cafe libraries.
Gitignored; not part of the build by default.

- `references/headers` — function addresses and names for USA v1.3.0; primary name source
- `references/decomp` — decompiled C++ for parts of the game (REDPRO2)
- `references/Cemu` — Cemu emulator source (Cafe OS / GX2 HLE behaviour)
- `references/sead` — sead engine and related EAD libraries used by NSMBU
- `references/Eft` — NintendoWare Eft (particles) as in NSMBU
- `references/G3d` — NintendoWare G3d (3D) as in NSMBU
- `references/FFL` — Wii U Face Library (Mii) as used by NSMBU
- `references/Lyt` — layout library reference
- `references/Snd` — sound library reference
- `references/Sys` — system library reference
- `references/Nintendo-File-Formats` — Nintendo file format documentation

Use `references/headers` first for addresses and names.
Use `references/decomp` for function bodies and layouts when a header is only a declaration.
Use `references/Cemu` for HLE behaviour, not for game symbol names.
