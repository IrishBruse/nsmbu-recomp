# Recomp mod manager

The mod manager installs, enables, and configures packages from inside the game.
No game content is part of the repository or of published packages.
Mods that need game data read it from the player's own game files.

This page describes the manager shell as it works today.

## Package kinds

| Kind | Status | Doc |
| --- | --- | --- |
| `content` | Live | [content.md](content.md) |
| `cemu` | Live | [cemu.md](cemu.md) |
| `native` | Rejected | [../deprecated/native-sdk-v1.md](../deprecated/native-sdk-v1.md) (archive) |
| `guest` | Rejected | [../deprecated/mod-sdk-v2.md](../deprecated/mod-sdk-v2.md) (archive) |
| `settings` | Rejected | — |

A proposal to move scripting and file replacement behind Lua is in [lua-mods.md](lua-mods.md).
Cemu graphics packs stay under that proposal.

## Player workflow

Open the in-game settings overlay (F1, Fn+F1 on many Macs, Cmd+, or Settings in the menu) and select **Mods**.
The tab manages installed packages.
There is no built-in gameplay catalogue and no cheat list.

The Installed packages section accepts a local folder or `.nsmbumod` ZIP.
Choose it with the file/folder picker, then press Install package.
Installed packages start disabled.
Nothing from a package is loaded until you enable it.
Enabling resolves required dependencies.
Missing versions, cycles, and declared conflicts produce an error.
The details show metadata, status, and bool/number/string/enum options.
String edits commit with Enter.
Disable a package and wait for its next game update before updating or removing it.
Content and Cemu packs also need a restart after enable, disable, or preset changes.
Reinstall the same ID while disabled to update; configuration is preserved by ID.
Refresh discovers manual folder changes when all packages are disabled.

Profiles save package toggles and configuration.
An older `profiles.json` may still contain `builtins` and `builtin_options`.
The loader ignores those keys.
The next save removes them.
Clone current creates another profile; select it in Active profile.
Switch away before deleting a profile.
Disable all covers installed packages.
Explicit startup environment values, including zero, override saved choices at startup.

Storage is `<host config directory>/ModManager`: `Mods/<id>/manifest.json` plus package files, and `profiles.json`.
`NSMBU_MOD_MANAGER_DIR` selects isolated storage.
`NSMBU_NO_HOST_INPUT` skips user preferences and package storage unless an explicit manager directory is supplied for a test.
Content mods are copied locally into manager storage; the original game files are preserved.
Game assets and saves are never committed, uploaded, or redistributed.

Test aids (only with `NSMBU_NO_HOST_INPUT`): `NSMBU_TEST_MOD_ENABLE=<id>` ticks that package's checkbox once when the Mods tab is drawn (with `NSMBU_TEST_OVERLAY=open:mods`).
It needs an explicit `NSMBU_MOD_MANAGER_DIR`, so it never applies to a player's storage.

## Shared manifest fields

| Field | Meaning |
| --- | --- |
| `format_version` | `1` |
| `id`, `name`, `version` | Stable lowercase ASCII identifier, display name, three-part version |
| `game_id` | `nsmbu-usa` (the runtime also verifies its exact RPX entry) |
| `author`, `description` | Optional display metadata |
| `minimum_manager_version` | Optional three-part minimum |
| `kind` | `content` or `cemu`. `native`, `guest`, and `settings` are rejected |
| `dependencies` | Objects with `id` and optional `minimum_version`. `builtin:<id>` is rejected |
| `conflicts` | Package IDs. `builtin:<id>` is rejected |
| `options` | Typed defaults and names; numeric min/max/step or enum choices |

Kind-specific fields:

| Field | Kind | Doc |
| --- | --- | --- |
| `content_dir` | `content` | [content.md](content.md) |
| `cemu_dir` | `cemu` | [cemu.md](cemu.md) |

The desktop folder and file install workflow is the supported UI.
Online downloads and catalogues are outside this manager.

## Packaging a mod

A package is a folder, or a ZIP archive of that folder's contents renamed to `.nsmbumod`, with `manifest.json` at its root.
Packages you publish must not contain game files; content and Cemu packs are imported locally by the player.
See [content.md](content.md) and [cemu.md](cemu.md).

## Validation

`mod_packages` exercises install, profiles, missing dependencies, configuration, disable, and removal.
A package that names `builtin:` fails to install.
The next profile save drops leftover `builtins` keys.
The same test installs synthetic Cemu and content packs: legacy imports, loose `.pack` files, preset validation, replacement conflicts, rejection of code/shader/rule/region mismatches, and the restart-only lifecycle.
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
