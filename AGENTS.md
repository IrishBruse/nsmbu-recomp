# NSMBU native port

This repository is a native port of New Super Mario Bros. U, USA version 1.3.0 (title `00050000-10101d00`, title version 64).
`tools/recomp/` translates `red-pro2.rpx` to C ahead of time.
`runtime/` implements the Cafe OS calls and the GX2 API as native code.
Linux and Windows render with Vulkan.
macOS renders with Metal.
The tree holds the runtime, the recompiler, and the installer.
The game dump, the keys, and the extracted files stay in `game/` on the machine that owns them.

The last merged base from ZeldaWWHDRecomp is recorded in `README.md` and `docs/nsmbu.md`.
A newer upstream commit comes in as one merge.
Follow `.agents/skills/upstream-merge/SKILL.md` when you take that merge.

## Build

Install `just`, CMake 3.20 or newer, Python 3, and Clang.
The generated C uses `musttail`, so CMake requires Clang.
On Linux, Clang links a GNU `libstdc++` (for example `libstdc++-14-dev` on Ubuntu).
`tools/build.py` adds the matching library path when it finds `libstdc++.so` under `/usr/lib/gcc/`.

`just --list` prints every recipe.
The developer table in `README.md` describes the same recipes.

- `just extract` — check `game/update/` for USA 1.3.0, extract `game/game.wux`, apply the update.
- `just build` — configure `build/` as Debug and build `nsmbu`.
- `just build-release` — build in Release (the configuration a release uses).
- `just build-sanitizer` — build with AddressSanitizer and UBSan.
- `just recomp` — translate `game/code/red-pro2.rpx` into `build/gen`.
- `just launch` — start `build/nsmbu` with `--game game/` and `--save nsmbu/save`.
- `just run` — `just build`, then `just launch`.
- `just run 3` — same, and load save slot 3 (slots are 1 through 5).
- `just run-release` — Release build, then a launch with the debug defaults off.
- `just launch-trace` — start with `--trace` for HLE logging.
- `just gdb` / `just lldb` — start under that debugger.

`just build` runs the recompiler when `game/code/red-pro2.rpx` is newer than `build/gen`.
When that RPX is absent, the build fills `build/gen` with `tools/recomp/stubgen.py` so the tree still links.
An RPX from a disc that lacks the 1.3.0 update has different addresses.
Those addresses will miss `references/headers` and `tools/recomp/headers_names.txt`.

The justfile exports `NSMBU_USER_DIR` to the gitignored `nsmbu/` folder.
`just launch` sets `NSMBU_PROFILE`, `NSMBU_VK_STATS`, `NSMBU_SYNC_STATS`, and `NSMBU_CRASH_RECOVERY` when those variables are unset.
`just launch-release` leaves those variables unset.
`just launch` and `just run` symlink each package under `modding/examples/` into `nsmbu/mods/<id>/`.
Existing real folders in that mods directory are left alone.
Set `NSMBU_NO_EXAMPLE_MODS=1` to skip those links.

The dump layout is in `game/README.md`.

## Tests

Host tests are CTest targets declared in `CMakeLists.txt`.
Run them from a configured build directory.

```bash
ctest --test-dir build --output-on-failure
```

Linux CI configures `build/linux` with Ninja and placeholder guest code from `tools/recomp/stubgen.py`, then runs that `ctest` plus these scripts.

```bash
python3 tools/installer/test_setup.py
python3 tools/recomp/test_builds.py
```

`just lint` type-checks the example Luau mods with `luau-lsp` and `modding/sdk/nsmbu.d.luau`.

A boot that dumps frames follows `.agents/skills/scripted-run/SKILL.md`.
A profile follows `.agents/skills/perf/SKILL.md` when the task is a profile.
A player-facing changelog summary follows `.agents/skills/changelog-summary/SKILL.md`.
A versioned GitHub release (CHANGELOG cut, `v*` tag, watch CI) follows `.agents/skills/release/SKILL.md`.
`tools/git/pre-push` runs a full local `ctest` before push (path-alias tests such as `mod_content_startup` first).

## Architecture

Each guest function is `void f_XXXXXXXX(Cpu* c)`.
`XXXXXXXX` is the address in USA v1.3.0 `red-pro2.rpx`.
That address is the id in hooks and in the runtime.
A regional rebuild shifts addresses through the map in `docs/builds.md` and keeps these symbol names.
Guest address `ea` is the host address `PPC_MEM_BASE + ea` (`0x200000000000` in `runtime/include/ppc.h`).
Loads and stores are host accesses plus a byte swap.
One guest thread runs on an emulated core at a time.
`runtime/src/threads.cpp` applies Cafe OS priority on host threads.

### Recompiler — `tools/recomp/`

`recomp.py` writes `build/gen/code_*.c`, `table.c`, and `imports.c`.
`hooks.txt` and `hooks_*.txt` name the call sites the runtime replaces.
`guest_stubs.py` writes weak stubs for symbols the runtime still imports.
CMake runs that script on every build.
A generated game function with the same name replaces the stub.
`names.py` refreshes `headers_names.txt` from a local `references/headers` checkout.
`recomp.py` reads the names file, so a normal build needs no headers checkout.

### Runtime — `runtime/`

- `main.cpp`, `core.cpp` — process start and the guest CPU loop.
- `threads.cpp` — Cafe OS threads, cores, and scheduling.
- `hle/` — Cafe OS imports this game calls (file system, AX audio, pad, software keyboard, heaps, zlib).
- `gx2/` — GX2 entry points. The register file matches the Latte GPU. Draws become commands for a host render thread.
- `gfx/` — presentation. Metal sources are `*.mm`. Vulkan sources are `gfx/vulkan/`.
- `overlay/` — the settings UI (Graphics, Display, Audio, Controls).
- `mods/` — mod packages, Cemu-style packs, and the Luau runtime (`mods/sdk/` is the native side of the mod API).
- `savestate.cpp`, `portable_state.cpp` — save-state slots and crash recovery.
- `third_party/cemu` — the Latte shader decompiler, AddrLib tiling, and register definitions. Behaviour notes for that code live in `references/Cemu`.

`docs/how-it-works.md` is the long form of this split.
`docs/vulkan.md` covers the Vulkan backend.
`docs/modding/README.md` covers mods.

### Other trees

- `tools/build.py`, `tools/launch.py`, `tools/extract.py` — the `just` recipes.
- `tools/wudextract/`, `tools/installer/` — disc extract and the setup window (`nsmbu-setup`).
- `modding/` — Luau SDK (`modding/sdk/`) and example mods.
- `android/` — the Android build.
- `cmake/` — Luau, Windows dependencies, and build info.

### CMake

The game target is `nsmbu`.
`gamecode` is the generated C.
`cemu_latte` is the vendored decompiler.
`nsmbu-setup` builds when `NSMBU_SETUP_GUI` is `ON`.
Each `*_test` executable is a CTest test.

- `NSMBU_RENDERER` — `BOTH`, `METAL`, or `VULKAN`. Default is `BOTH` on macOS and `VULKAN` elsewhere. Metal builds on macOS only.
- `GEN_DIR` — recompiler output. Default is `build/gen`.
- `NSMBU_SETUP_GUI` — build the graphical installer. Default is `OFF`.
- `NSMBU_BUNDLED_DEPS` — build missing dependencies from pinned sources. Default is `OFF`.
- `NSMBU_VERSION` — version string in logs. Default comes from `git describe`.

## Code style

Match the file you are editing.
Runtime code is C++20.
Generated guest code is C11.
Put the opening brace on the same line as the declaration.
Indent with 4 spaces.
The `nsmbu` target compiles with `-Wall`.
`gamecode` and `cemu_latte` compile with warnings off, because that code is generated or vendored.

## Names and references (`references/`)

These checkouts are gitignored.
They are not part of the build.

Every address in this repository is an address in USA v1.3.0 `red-pro2.rpx`.
Keep that address as the key.
Put the name beside it.
`docs/names.md` is the map from header names to the names this port uses.

- `references/headers` — class and function names for USA v1.3.0. This is the name source. Each symbol has an `Address:` comment.
- `references/decomp` — decompiled C++ for part of the game (REDPRO2). Use it for a function body, member order, or layout when the header is only a declaration.
- `references/Cemu` — Cemu source for Cafe OS and GX2 high-level behaviour.
- `references/sead` — the sead engine and related EAD libraries used by NSMBU.
- `references/Eft` — NintendoWare Eft (particles) as in NSMBU.
- `references/G3d` — NintendoWare G3d (3D) as in NSMBU.
- `references/FFL` — the Wii U Face Library (Mii) as used by NSMBU.
- `references/Lyt` — layout library.
- `references/Snd` — sound library.
- `references/Sys` — system library.
- `references/Nintendo-File-Formats` — Nintendo file format notes.

Game symbol names come from `references/headers`.
High-level emulation behaviour comes from `references/Cemu`.

## Local userdata (`nsmbu/`)

`just` points `NSMBU_USER_DIR` at this gitignored folder.
A portable release uses the same layout under `data/nsmbu/`, selected by `portable.txt` next to the binary.

- `nsmbu/settings.ini` — overlay settings (Graphics, Display, Audio, and related keys).
- `nsmbu/controls.json` — controller bindings.
- `nsmbu/save/` — Wii U save (`user/`, `common/`). Launches pass `--save nsmbu/save`.
- `nsmbu/states/` — save-state slots and crash recovery.
- `nsmbu/logs/` — run log (`nsmbu.log`), the previous log, crash logs, and frame dumps.
- `nsmbu/shadercache/` — Vulkan pipeline and shader disk cache.
- `nsmbu/mods/` — installed mod packages.
- `nsmbu/profiles.json` — mod profile toggles and options.
