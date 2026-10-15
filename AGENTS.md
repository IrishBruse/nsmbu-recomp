# NSMBU native port

This repository is a native port of New Super Mario Bros. U, USA version 1.3.0 (title `00050000-10101d00`, title version 64).
`tools/recomp/` translates `red-pro2.rpx` to C ahead of time.
`runtime/` implements the Cafe OS calls and the GX2 API as native code.
Linux and Windows render with Vulkan.
macOS renders with Metal.
The game dump, the keys, and the extracted files stay in `game/` on the machine that owns them.
The dump layout is in `game/README.md`.

The last merged base from ZeldaWWHDRecomp is recorded in `README.md` and `docs/nsmbu.md`.
Follow `.agents/skills/upstream-merge/SKILL.md` when you take a newer upstream commit.

## Build

The generated C uses `musttail`, so the build uses Clang.
On Linux, `tools/build.py` adds the `libstdc++` path when it finds `libstdc++.so` under `/usr/lib/gcc/`.

`just build` runs the recompiler when `game/code/red-pro2.rpx` is newer than `build/gen`.
When that RPX is absent, the build fills `build/gen` with `tools/recomp/stubgen.py` so the tree still links.
An RPX from a disc that lacks the 1.3.0 update has different addresses.
Those addresses miss `references/headers` and `tools/recomp/headers_names.txt`.

`just launch` sets `NSMBU_PROFILE`, `NSMBU_VK_STATS`, `NSMBU_SYNC_STATS`, and `NSMBU_CRASH_RECOVERY` when those variables are unset.
`just launch-release` leaves those variables unset.
`just launch` and `just run` symlink each package under `modding/examples/` into `nsmbu/mods/<id>/`.
Existing real folders in that mods directory stay in place.
Set `NSMBU_NO_EXAMPLE_MODS=1` to skip those links.

## Tests

Host tests are CTest targets in `CMakeLists.txt`.

```bash
ctest --test-dir build --output-on-failure
```

Linux CI also runs these scripts after that `ctest`.

```bash
python3 tools/installer/test_setup.py
python3 tools/recomp/test_builds.py
```

`tools/git/pre-push` runs a full local `ctest` before a push.

A profile follows `.agents/skills/perf/SKILL.md`.

## Guest code

Each guest function is `void f_XXXXXXXX(Cpu* c)`.
`XXXXXXXX` is the address in USA v1.3.0 `red-pro2.rpx`.
That address is the id in hooks and in the runtime.
A regional rebuild shifts addresses through the map in `docs/builds.md` and keeps these symbol names.
Guest address `ea` is the host address `PPC_MEM_BASE + ea` (`0x200000000000` in `runtime/include/ppc.h`).
Loads and stores are host accesses plus a byte swap.
One guest thread runs on an emulated core at a time.
`runtime/src/threads.cpp` applies Cafe OS priority on host threads.

`tools/recomp/recomp.py` writes `build/gen/code_*.c`, `table.c`, and `imports.c`.
`hooks.txt` and `hooks_*.txt` name the call sites the runtime replaces.
`guest_stubs.py` writes weak stubs for symbols the runtime still imports.
CMake runs that script on every build.
A generated game function with the same name replaces the stub.
A normal build reads `tools/recomp/headers_names.txt` from git.
`names.py` refreshes that file from a local `references/headers` checkout.

Read `docs/how-it-works.md` for the runtime split.
Read `docs/vulkan.md` when you change the Vulkan backend.
Read `docs/modding/README.md` when you change mod load, content redirects, or the Luau API.

## Code style

Match the file you are editing.
Put the opening brace on the same line as the declaration.
Indent with 4 spaces.
The `nsmbu` target compiles with `-Wall`.
Warnings in `gamecode` and `cemu_latte` are expected.
That code is generated or vendored.

## Names

When you name a guest symbol, read `docs/names.md`.
Keep the USA v1.3.0 address as the key.
Put the name beside it.
`references/` checkouts are gitignored and are not part of the build.
When the task is in sead, particles, G3d, Miis, layouts, sound, or a Nintendo file format, open the matching tree listed in `docs/names.md`.

## Local userdata

`just` points `NSMBU_USER_DIR` at the gitignored `nsmbu/` folder.
A portable release uses the same layout under `data/nsmbu/` when `portable.txt` sits next to the binary.
The run log is `nsmbu/logs/nsmbu.log`.
`nsmbu/mods.json` stores the enabled mods and their options.
