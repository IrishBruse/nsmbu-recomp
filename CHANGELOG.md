# Changelog

Add new notes under Unreleased.
At release, move those notes to a version heading and leave Unreleased empty.

## Unreleased

-   **Change**: Lifecycle callbacks live on `nsmbu.hooks` as assignable fields (`on_logic_step`, `on_config_changed`, `on_loaded`, `on_unload`) instead of `nsmbu.on_*` register helpers and `unlisten`.
-   **Add**: Mod API `nsmbu.scene`: boot-flow phase probes (`title`, `file_select`, `map_phase`, `course`, `course_select`, `phase`) and transitions (`open_title`, `open_file_select`, `open_course_select`, `confirm_file`, `load_story`) that call `Scene::setNextScene` / `Scene::setCourse_Story` inside the port.
-   **Add**: Mod API `nsmbu.hooks.on_loaded` fires once right after a package is loaded, as soon as the game can be called into.
    `nsmbu.input.press(...)` injects buttons for N logic steps.
    Example `example.intro-skip` loads to the world map without input: from `on_logic_step` it calls `nsmbu.scene.open_file_select`, `confirm_file`, and `open_course_select` in order, so it never presses buttons or reads guest memory.
-   **Docs**: `docs/names.md` maps the function names in `references/headers` and `references/decomp` to the guest addresses and the port names (`f_…`, `hook_…`, `GC`/`GD`, `kName`).
    The reference notes now point at `references/headers` instead of the old `headers/` path.
    The scene phase constants in the mod SDK now read the canonical address through `GC`, and the doc table gains `sead::TTaskFactory<TitleScene>` and `sead::TTaskFactory<CourseSelectTask>`.
-   **Add**: The generated C names its functions: `tools/recomp/names.py` extracts the addresses and names from the [nsmbu/headers](https://github.com/nsmbu/headers) project into `tools/recomp/headers_names.txt` (in git), and `recomp.py` writes the header name in a comment above the definition, next to the prototype in `funcs.h`, and at every call site of a named function. `stubgen.py` writes the prototype comments. The `f_XXXXXXXX` symbol stays the key.

## 0.3.0

-   **Add**: Luau mod packages: mods are Luau scripts (`kind: luau`) with the host API, logic-step callbacks, optional `content_dir` content, and hot reload.
    The mod SDK, type definitions, and example packages live under `modding/`.
    The old guest and native SDKs are removed.
-   **Add**: Runtime level editing for Luau mods: `nsmbu.level.actors()` lists live actors and `nsmbu.level.spawn(...)` spawns them.
    Example `example.level-editor` shows both.
-   **Add**: High-level game helpers `nsmbu.level` and `nsmbu.player` for cheats and game state.
-   **Add**: Settings overlay Audio tab: mute, volume, and TV / GamePad / Auto output, saved in `settings.ini`.
-   **Add**: Performance overlay detail (lag breakdown) shows bar rows for GPU, renderer CPU, vsync, waits, and idle, sorted by cost.
-   **Change**: Faster guest texture decode (Vulkan) and parallel save-state memory capture and restore.
    Loading slot 3 drops from 262 ms to 70-85 ms.
-   **Change**: Faster recompiler codegen from [ZeldaWWHDRecomp#83](https://github.com/ZeldaWWHDRecomp/ZeldaWWHDRecomp/pull/83) by depende3000 (SwitchWakerHD).
    Off switches use `NSMBU_RECOMP_*`.
-   **Change**: The settings overlay opens with `` ` `` or F12.
    F1-F5 load save-state slots.
-   **Change**: Every `just` recipe puts settings, states, mods, and shader cache in gitignored `user/`.
    Example mods are linked into the dev user dir, not copied.
    `just run` no longer mutes audio.
-   **Fix**: The port is USA-only now: build registry, `hooks.txt`, `just extract`, and the installer all use the USA 1.3.0 update.
    Wind Waker HD leftovers are removed.
-   **Fix**: Boot and draw-queue crashes are fixed, and the overworld path seam is fixed.
-   **Docs**: Live modding docs live under `docs/modding/`.

## 0.2.0

-   **Fix**: The crash in World 1-1 is fixed.
-   **Play**: World 1-1 through 1-3 are playable.
    Performance is low and frame rate often stays below 60 fps.

## 0.1

-   **Add**: The download is the NSMBU runtime and installer.
    It has no game files and no game code.
    The first start builds the game from your dump.
-   **Add**: The frame menu offers 60, 120, 165, and 240 fps.
    Game logic stays at 60 steps a second.
-   **Change**: Vulkan presents the TV and the GamePad on one vsync.
