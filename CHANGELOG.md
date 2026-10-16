# Changelog

Add new notes under Unreleased.
At release, move those notes to a version heading and leave Unreleased empty.

## Unreleased

-   **Fix**: Scanning a dropped Cemu content or graphics pack no longer writes `manifest.json` into the player's pack folder.
    Synthesized metadata stays in memory until install stages a managed package.
-   **Fix**: `open_file_select` no longer calls `0x0297D58C` and no longer creates menu factory `0x0202B61C`.
    Creating that factory early shows the time-attack trial layout.
    `0x0297D58C` enters the challenge-select state whose initialize opens `ChallengeSelectScene`.
    The call now selects save file 0 on an existing menu and enters file state `0x0297D268`.
-   **Change**: `example.intro-skip` calls `open_first_save` once the title or map phase is current.
    That call writes save file 0 and runs guest `0x029872BC`, which requests the world map when the file has a record.
    It does not press buttons and it does not create menu factory `0x0202B61C`.
    The scene source is `*(*(CourseTask) + 0x10)`, the same pointer the title confirm passes to `setNextScene`.
    An empty file 0 follows the game branch to factory `0x0202B570`.
    The call applies save file 0 with guest `0x029D7648` before `0x029872BC`.
    It then sets `MainGame.mIsSinglePlayerDRC` so the map reads the GamePad or the Pro Controller.
-   **Fix**: The error viewer closes as soon as the game shows it, and that call does not pause the game.
    This port does not draw that viewer.
    A pause held the world map on the opening circle and made the map run slow.
-   **Change**: Mod storage is `<host config>/mods.json` with top-level `enabled` and `config` (formatted JSON).
    Mod profiles are removed.
    An older nested `profiles` object or `profiles.json` still loads; the next save writes the flat file.

## 0.4.2

-   **Fix**: The setup window follows the system dark or light theme.
-   **Fix**: Setup buttons and long labels no longer clip; menu actions are centered and sized to the window.
-   **Fix**: The install progress screen gives most of the space to the steps and keeps the Details log readable.
-   **Change**: Translating the game code shows live progress in the installer.
-   **Add**: `just installer` builds a Ninja Release under `build/rel`, packages a real SDK, and opens setup (for local install tests).

## 0.4.1

-   **Fix**: The release installer ships `tools/recomp/names.py` and `headers_names.txt`, so translating the game code no longer fails with `ModuleNotFoundError: No module named 'names'`.

## 0.4.0

-   **Change**: Mod packages live in `<host config>/mods/` and profiles in `<host config>/profiles.json`.
    The old `ModManager/` tree is removed with no migration.
    Package folder names are not required to match the manifest `id`.
-   **Change**: Dev launches symlink each `modding/examples/` package into `nsmbu/mods/<id>/` instead of replacing the whole mods folder with one link.
    Real folders already in mods stay in place.
-   **Change**: The Mods tab drops the package picker, path field, Install, Refresh, and Hot reload controls.
    **Open mods folder** opens `mods`; players copy package folders there.
    The tab rescans about once per second while open when every package is disabled.
-   **Add**: Native Cemu content packs: scan detects Definition-only `rules.txt` + `content/` (no player `manifest.json`), ignores Cemu `meta/`, synthesizes a Luau `content_dir` package, and raises size caps to 512 MiB per file and 2 GiB per package ([docs/modding/cemu-content.md](docs/modding/cemu-content.md)).
-   **Change**: `example.intro-skip` waits on scene probes instead of fixed step counts.
    The first call opens save slot 0.
    It does not open the time-attack scene.
    The first call also runs while `map_phase()` is true, because a boot leaves the current scene on AreaTask `0x020292B8` after it requests TitleScene.
    `open_file_select` requests menu factory `0x0202B61C`, then changes that task's state.
    It does not call the state change on the flow task.
    The scene calls pass the flow task as the `setNextScene` source.
    They do not pass the `Scene` object.
    `open_file_select` returns `"scene busy"`, `"menu missing"`, `"state not ready"`, or `"file not ready"` when it cannot run.
    The script sends the same call again.
    The script stops when `file_index()` is 0.
-   **Change**: Lifecycle callbacks live on `nsmbu.hooks` as assignable fields (`on_logic_step`, `on_config_changed`, `on_loaded`, `on_unload`) instead of `nsmbu.on_*` register helpers and `unlisten`.
-   **Add**: Mod API `nsmbu.scene`: boot-flow phase probes (`title`, `file_select`, `map_phase`, `course`, `course_select`, `phase`) and transitions (`open_title`, `open_file_select`, `open_course_select`, `confirm_file`, `load_story`) that call `Scene::setNextScene` / `Scene::setCourse_Story` inside the port.
-   **Add**: Mod API `nsmbu.hooks.on_loaded` fires once right after a package is loaded, as soon as the game can be called into.
    `nsmbu.input.press(...)` injects buttons for N logic steps.
    Example `example.intro-skip` stops on save slot 0 without input.
    From `on_logic_step` it calls `nsmbu.scene.open_file_select`, then it reads `file_index()`.
    It does not open the time-attack scene.
    It never presses buttons or reads guest memory.
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
-   **Change**: Every `just` recipe puts settings, states, mods, and shader cache in gitignored `nsmbu/`.
    Example mods are linked into the dev userdata dir, not copied.
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
