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

## What is still Wind Waker

These parts still describe Wind Waker HD.

Do not treat them as NSMBU behaviour.

- `tools/recomp/nsmbu_hooks/` holds Wind Waker function addresses.
  The recompiler does not load that directory.
  A hook address from Wind Waker is a different function in NSMBU.
- Save tools under `tools/savegame/` read `cking.sav`.
- Language packs and many mods follow Wind Waker file names.
- `CMakeLists.txt` still uses the Wind Waker project name.
  The build fails until `build/gen/code_*.c` exists.

## Game files

Put the disc image and `keys.txt` in `disc/`.

Git ignores that folder except `disc/.gitkeep`.

`just extract` reads that folder and writes `game/`.

`just build` configures `build/` with Clang and builds the `nsmbu` target.

If `build/gen` is missing, `just build` runs `stubgen.py` there so the tree links without your RPX.

Clang needs a GNU `libstdc++` (for example `libstdc++-14-dev` on Ubuntu).
`tools/build.py` adds the matching `-L` path when it finds `libstdc++.so` under `/usr/lib/gcc/`.

`just run` runs `build/nsmbu` with `--game` set to `game/`.

Built-in mods, the mod manager, and cheats are off in this port (`mods::mods_enabled()` is false unless you build with `-DNSMBU_MODS_ENABLED`).

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

`runtime/src/nsmbu_guest_stubs.c` holds a weak stub for each Wind Waker function the runtime still names.

Regenerate it with `python3 tools/recomp/guest_stubs.py`.

The NSMBU executable does not call those functions.

A generated game function with the same name replaces the stub.

The first run will miss Cafe OS imports that Wind Waker never calls.

Add those imports in `runtime/src/hle/` after the recompiler report lists them.

`build/gen/imports.json` is that list.
