# NSMBU port notes

## Upstream base

Use this commit when you rebase onto a newer Wind Waker HD recomp.

Repository: https://github.com/ZeldaWWHDRecomp/ZeldaWWHDRecomp

Branch: `main`

Commit: `853d7b18c8c6703c5fc50c40cb923c1fb9503ecd`

Subject: `README: v0.2.8 notes, shorter language-source line`

Author date: Thu Oct 8 13:37:16 2026 +0200

Local `main` points at that commit.

The remote name in this clone is `upstream`.

This clone is shallow.

It stores that commit only.

Fetch the full history before a rebase.

```bash
git fetch --unshallow upstream
git fetch upstream main
git rebase --onto upstream/main 853d7b18c8c6703c5fc50c40cb923c1fb9503ecd
```

## What you can reuse

The Wind Waker HD port and NSMBU are both Wii U titles.

`tools/recomp/recomp.py` turns one RPX into C.

`runtime/` implements Cafe OS calls and GX2 on Vulkan.

On Linux the build uses Vulkan.

The executable name in the runtime is `red-pro2.rpx`.

That file is `code/red-pro2.rpx` in the NSMBU filesystem.

The public headers in `headers/` match NSMBU v1.3.0.

Each function comment in those headers has a guest address.

## Fork leftovers

These parts still need NSMBU-specific work.

- `tools/recomp/nsmbu_hooks/` holds upstream guest addresses.
  The recompiler does not load that directory.
  A hook address from the upstream game is a different function in NSMBU.
- Language packs and many mods still follow upstream Cemu pack folder names.
- `runtime/src/nsmbu_guest_stubs.c` holds weak stubs for symbols the runtime still names from upstream.
  Regenerate with `python3 tools/recomp/guest_stubs.py`.

The upstream built-in mods and cheats are removed.

## Game files

User files live under `game/`:

- `game/game.wux` — disc image
- `game/game.key` — title key next to the image
- `game/common.key` — common key (or `WIIU_COMMON_KEY`)
- `game/code/`, `game/content/`, `game/meta/` — output of `just extract`

`.gitignore` ignores all of `game/` except `game/README.md` (layout and steps for this folder).

`just extract` reads `game/game.wux` and writes into the same folder tree.

`just build` configures `build/` in **Debug**, links `compile_commands.json` at the repo root, and builds `nsmbu`.

If `game/code/red-pro2.rpx` exists, `just build` runs `recomp.py` into `build/gen` when the RPX is newer than the generated code.

Otherwise it runs `stubgen.py` so the tree links without your RPX.

`just build-release` uses **Release** with no debug symlink.

`just build-sanitizer` adds AddressSanitizer and UBSan.

`just build --mods` passes `-DNSMBU_MODS_ENABLED=ON`.

Clang needs a GNU `libstdc++` (for example `libstdc++-14-dev` on Ubuntu).
`tools/build.py` adds the matching `-L` path when it finds `libstdc++.so` under `/usr/lib/gcc/`.

`just run` starts `build/nsmbu` with `--game` set to `game/`, loads `env-debug.txt` and `env.txt` when present, and sets default debug env vars (`NSMBU_PROFILE`, `NSMBU_VK_STATS`, `NSMBU_SYNC_STATS`, `NSMBU_CRASH_RECOVERY`).

Copy [`env-debug.example`](env-debug.example) to `env-debug.txt` to add more.

`just run-release` skips those defaults.

`just run-trace` passes `--trace` for HLE logging.

`just gdb` and `just lldb` run the game under a debugger.

`just debug` runs `just build` then `just run`.

`just recomp` runs `recomp.py` only.

The package manager is off in this port (`mods::mods_enabled()` is false unless you build with `-DNSMBU_MODS_ENABLED`).

Put an extracted game you own in `game/`.

The layout is `game/code/`, `game/content/`, and `game/meta/`.

The headers match the v1.3.0 executable.

A disc image without that update has a different `red-pro2.rpx`.

Addresses in `headers/` will not match that older file.

`game/` and `*.rpx` are gitignored.

## Recompile

Use Clang.

The generated C uses `musttail`.

```bash
python3 tools/recomp/recomp.py game/code/red-pro2.rpx build/gen
cmake -S . -B build -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```

The NSMBU executable does not call the guest stub symbols.

A generated game function with the same name replaces the stub.

The first run will miss Cafe OS imports the upstream title never called.

Add those imports in `runtime/src/hle/` after the recompiler report lists them.
