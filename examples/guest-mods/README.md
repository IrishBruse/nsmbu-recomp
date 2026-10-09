# Example guest mods (Mod SDK v2 prototype)

Two small mods written in C and compiled for the game's CPU (32-bit big-endian PowerPC). See
[docs/mod-sdk-v2.md](../../docs/mod-sdk-v2.md) for the design. They contain no game code or data:
game functions are referenced by address only (see `runtime/guest/include/wwhd/functions.h` for
`red-pro2.rpx`).

| Mod | What it shows |
| --- | --- |
| `play-scene-ticker` | Entry and return hooks on the play-scene step (`dScnPly_Execute`, `025B0314`) and the `every` option. While a level is running, the mod logs a counter every `every` logic steps. |
| `smooth-step-replace` | A full replacement of `cLib_addCalc2` (`0200ED84`) with an equivalent implementation; every other call uses the game's code through `NSMBU_GAME_ORIGINAL`. Behaviour is unchanged; the log counts calls. |

Build (needs clang with the PowerPC target and ld.lld; on macOS `brew install llvm lld`):

```sh
make CLANG=/opt/homebrew/opt/llvm/bin/clang LLD=/opt/homebrew/opt/lld/bin/ld.lld
```

Each folder is then a package (`manifest.json` + `mod.elf`). The source includes the guest mod
SDK under `runtime/guest/include` (`nsmbu_guest.h`, `nsmbu/functions.h`, and generated addresses).

Install each folder through **Mods → Installed packages → Choose folder → Install package**,
enable it, accept the ELF trust confirmation, and restart. Set play-scene-ticker's `every` option
in the Mods tab. The manager builds and caches the translated modules on startup.
Game code must have been translated with `--mod-hooks`; this remains opt-in until the
phase 1 performance gate passes. Android guest modules are currently unsupported.
