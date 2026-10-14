# Recomp mod manager

The mod manager installs, enables, and configures packages from inside the game.
No game content is part of the repository or of published packages.
Mods that need game data read it from the player's own game files.

This page describes the manager shell as it works today.
Manager version **1.0.0**.

## Package kinds

| Kind | Status | Doc |
| --- | --- | --- |
| `luau` | Live | [luau-mods.md](luau-mods.md), [api.md](api.md), [content.md](content.md) for `content_dir`, [cemu-content.md](cemu-content.md) for drop-in Cemu level packs |
| `cemu` | Live | [cemu.md](cemu.md) |
| `content` | Alias this release (migrates to `luau` on scan) | [content.md](content.md) |

Live kinds are `luau` and `cemu`.
`kind: content` is accepted as a read-only alias this release and rewritten to `luau` on scan.
Cemu graphics packs stay separate from file-only Cemu content packs.
See [luau-mods.md](luau-mods.md) for the scripting and file-replacement plan.

## Player workflow

Open the in-game settings overlay (`` ` `` or F12, Cmd+, or Settings in the menu) and select **Mods**.
The tab manages packages that already sit in the mods folder.
There is no built-in gameplay catalogue and no cheat list.

**Open mods folder** opens `<host config directory>/mods` in the system file manager.
Put each package in that folder as its own subdirectory with a `manifest.json`, or drop in a Cemu content pack root ([cemu-content.md](cemu-content.md)).
The package `id` comes from the manifest; the folder name can differ.
There is no in-game import or install step.
The Mods tab rescans that folder about once per second while it is open, when every package is disabled (same rule as the old refresh).
Installed packages start disabled.
Nothing from a package is loaded until you enable it.
Enabling resolves required dependencies.
Missing versions, cycles, and declared conflicts produce an error.
The details show metadata, status, and bool/number/string/enum options.
String edits commit with Enter.
Disable a package and wait for its next game update before replacing or removing its folder.
Luau packages with `content_dir`, and Cemu packs, need a restart after enable, disable, or preset changes.
Script-only Luau packages (no `content_dir`) load live on enable.
Optional hot reload for script-only packs is the top-level `hot_reload` boolean in `profiles.json` (not shown in the Mods tab).
When it is on, enabled script-only packs unload and reload when `.luau` files under the package change.
Packs with `content_dir` ignore that path and still need a restart.
Replace the same folder id while disabled to update; configuration is preserved by ID.

Profiles save package toggles and configuration.
An older `profiles.json` may still contain `builtins` and `builtin_options`.
The loader ignores those keys.
The next save removes them.
Clone current creates another profile; select it in Active profile.
Switch away before deleting a profile.
Disable all covers installed packages.
Explicit startup environment values, including zero, override saved choices at startup.

Storage is `<host config directory>`: `mods/<folder>/manifest.json` plus package files, and `profiles.json`.
`NSMBU_MOD_MANAGER_DIR` selects isolated storage (same layout under that directory).
`NSMBU_NO_HOST_INPUT` skips user preferences and package storage unless an explicit manager directory is supplied for a test.
Source `just launch` / `just run` symlink each `modding/examples/` package into `nsmbu/mods/<id>/`.
Real folders already in mods are not replaced.
Set `NSMBU_NO_EXAMPLE_MODS=1` to skip those links.
Package files live under the mods folder; the original game files are not changed.
Game assets and saves are never committed, uploaded, or redistributed.

Test aids (only with `NSMBU_NO_HOST_INPUT`): `NSMBU_TEST_MOD_ENABLE=<id>` ticks that package's checkbox once when the Mods tab is drawn (with `NSMBU_TEST_OVERLAY=open:mods`).
It needs an explicit `NSMBU_MOD_MANAGER_DIR`, so it never applies to a player's storage.

## Shared manifest fields

| Field | Meaning |
| --- | --- |
| `format_version` | `1` or `2` |
| `id`, `name`, `version` | Stable lowercase ASCII identifier, display name, three-part version |
| `game_id` | `nsmbu-usa` (the runtime also verifies its exact RPX entry) |
| `author`, `description` | Optional display metadata |
| `minimum_manager_version` | Optional three-part minimum |
| `kind` | `luau` or `cemu`. `content` is accepted as an alias this release. `native`, `guest`, and `settings` are rejected |
| `dependencies` | Objects with `id` and optional `minimum_version`. `builtin:<id>` is rejected |
| `conflicts` | Package IDs. `builtin:<id>` is rejected |
| `options` | Typed defaults and names; numeric min/max/step or enum choices |

Kind-specific fields:

| Field | Kind | Doc |
| --- | --- | --- |
| `luau` (`api_version`, `entry`) | `luau` | [luau-mods.md](luau-mods.md). `entry` optional when `content_dir` is set |
| `content_dir` | `luau` (also on legacy `content` alias) | [content.md](content.md) |
| `cemu_dir` | `cemu` | [cemu.md](cemu.md) |

A Luau package requires at least one of `luau.entry` or `content_dir`.
The desktop folder and file install workflow is the supported UI.
Online downloads and catalogues are outside this manager.

## Packaging a mod

A package is a folder, or a ZIP archive of that folder's contents renamed to `.nsmbumod`, with `manifest.json` at its root.
Packages you publish must not contain game files; content and Cemu packs are copied locally by the player into the mods folder.
See [luau-mods.md](luau-mods.md), [content.md](content.md), and [cemu.md](cemu.md).

## Validation

`mod_packages` exercises install, profiles, missing dependencies, configuration, disable, and removal.
A package that names `builtin:` fails to install.
The next profile save drops leftover `builtins` keys.
The same test installs synthetic Cemu and Luau content packs: legacy imports, loose `.pack` files, preset validation, replacement conflicts, rejection of code/shader/rule/region mismatches, and the restart-only lifecycle.
Install of `kind: native`, `kind: guest`, and `kind: settings` is asserted to fail.

`mod_content_startup`, `mod_cemu_startup`, and `mod_cemu_backend` start the manager on prepared storage and check startup activation.
`mod_content_fs_off` and `mod_content_fs_on` call the guest filesystem HLE handlers against synthetic files.
`mod_cemu` covers `rules.txt` parsing.
All fixtures are synthetic; these are host tests, not proof that a given community mod looks right in game.

## Public project references (checked 2026-10-06)

- [Zelda64Recomp](https://github.com/Zelda64Recomp/Zelda64Recomp): built-in manager UX reference.
- [Its mod template](https://github.com/Zelda64Recomp/MMRecompModTemplate/blob/main/mod.toml): manifest shape reference.
- [BlueWake mod documentation](https://github.com/chrissotraidis/bluewake/blob/main/docs/MODS.md): architectural research only.
- [ModernGekko](https://github.com/ExpansionPak/ModernGekko): loader research only; packages are not ABI-compatible here.

No source from those projects has been copied into this mod manager.
