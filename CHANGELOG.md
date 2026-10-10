# Changelog

Add new notes under Unreleased.
At release, move those notes to a version heading and leave Unreleased empty.

## Unreleased

-   **Docs**: The README records the last ZeldaWWHDRecomp merge commit.
    The upstream-merge skill updates that note.
-   **Remove**: Guest Mod SDK v2 and Native SDK v1 are gone (`examples/guest-mods/`, `tools/guestmod/`, the runtime guest loader, `code_mods`, `--mod-hooks` emission, the `guestmods` CI workflow, and the Native SDK v1 C ABI path).
    The mod manager rejects guest and native packages.
-   **Docs**: Live modding docs live under `docs/modding/` (manager, content, cemu, Lua proposal).
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
