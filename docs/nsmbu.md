# NSMBU port notes

## Upstream base

Use this commit as the last Wind Waker HD recomp commit in this tree.

Repository: https://github.com/ZeldaWWHDRecomp/ZeldaWWHDRecomp

Branch: `main`

Commit: `defb89f21607345e2e97b2c145a071a3f362640c`

Subject: `Recompiler: mulli, mullw and neg without signed overflow in the generated C (#82)`

Author date: Fri Oct 9 11:22:57 2026 +0200

The remote name in this clone is `upstream`.

The archived Wind Waker HD readme is [upstream-wwhd-readme.md](upstream-wwhd-readme.md).

The next update is one merge, not a rebase.

Follow `.cursor/skills/upstream-merge/SKILL.md`.

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

- Language packs and many mods still follow upstream Cemu pack folder names.
- The build writes weak stubs for symbols the runtime still names from upstream.
  CMake runs `tools/recomp/guest_stubs.py` and compiles the output.

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

Clang needs a GNU `libstdc++` (for example `libstdc++-14-dev` on Ubuntu).
`tools/build.py` adds the matching `-L` path when it finds `libstdc++.so` under `/usr/lib/gcc/`.

`just launch` starts `build/nsmbu` with `--game` set to `game/` and sets default debug env vars (`NSMBU_PROFILE`, `NSMBU_VK_STATS`, `NSMBU_SYNC_STATS`, `NSMBU_CRASH_RECOVERY`) when they are not already set in the environment.

`just launch-release` skips those defaults.

`just launch-trace` passes `--trace` for HLE logging.

`just gdb` and `just lldb` run the game under a debugger.

`just run` runs `just build` then `just launch`.

`just recomp` runs `recomp.py` only.

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
