# Changelog

Add new notes under Unreleased.
At release, move those notes to a version heading and leave Unreleased empty.

## Unreleased

-   **Remove**: `just build` no longer links `compile_commands.json` at the repository root.
-   **Fix**: The build registry uses NSMBU USA title `0005000010101d00` (1.3.0 / title version 64) and its RPX SHA-256.
    Wind Waker HD title ids and hashes are removed.
    The leftover Europe address map is removed; this port is USA-only for now.
-   **Add**: `just extract` and the installer require the USA 1.3.0 update (title version 64).
    Source builds use `game/update/`.
    The installer looks for `update/` next to the disc image (or `--update-dir` / the Keys screen), checks it, applies it after extract, and verifies the merged dump against the known USA SHA-256.
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
-   **Fix**: `hooks.txt` keeps only the USA 1.3.0 DistantViewMgr flicker site (`@022A79C4`).
    Wind Waker HD leftover entry points are removed so `just recomp` accepts `red-pro2.rpx`.


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
