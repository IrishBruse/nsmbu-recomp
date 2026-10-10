# Changelog

Add new notes under Unreleased.
At release, move those notes to a version heading and leave Unreleased empty.

## Unreleased

-   **Add**: Lua mod packages (`kind: lua`) with embedded LuaJIT, logic-step callbacks, and the phase-2 host API (config, log, status, guest memory).
    The mod manager accepts format 1 or 2, scans for forbidden binaries/shaders, and loads script-only Lua packages live each logic step.
    Package tests cover load, options, fault isolation, sandbox require, and guest memory (`mod_package_test --lua`).
-   **Add**: Lua phase 3 content moves: optional `content_dir` on `kind: lua` packages (content-only packages are valid; restart when `content_dir` is set).
    The importer emits `kind: lua`.
    `kind: content` remains a one-release read-only alias and migrates to `lua` on scan.
-   **Docs**: The README records the last ZeldaWWHDRecomp merge commit.
    The upstream-merge skill updates that note.
-   **Remove**: Guest Mod SDK v2 and Native SDK v1 are gone (`examples/guest-mods/`, `tools/guestmod/`, the runtime guest loader, `code_mods`, `--mod-hooks` emission, the `guestmods` CI workflow, and the Native SDK v1 C ABI path).
    The mod manager rejects guest and native packages.
-   **Docs**: Live modding docs live under `docs/modding/` (manager, content via Lua `content_dir`, cemu, Lua phases 2–3).
    SDK v1/v2 archives stay in `docs/deprecated/`.
-   **Remove**: `kind: settings` packages and the built-in settings manager stub are gone.
-   **Fix**: The overworld path seam is fixed.

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
