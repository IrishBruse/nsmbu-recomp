# File replacements (`content_dir` on Lua packages)

Status: **live**.
File replacements (models, textures, UI, language packs) ship as Lua packages with an optional `content_dir`.
`kind: content` is a one-release read-only alias and migrates to `kind: lua` on scan.
See [lua-mods.md](lua-mods.md) for the package model.
See [mod-manager.md](mod-manager.md) for install, profiles, and shared manifest fields.

Packages may be content-only: a manifest and a `content/` tree, with no script.
They do not run host or guest code unless they also ship a Lua entry.

## Player workflow

Use **Mods → Installed packages → Choose folder… / Choose package… → Install package**.
Select a single local pack with a `content/` directory, or its ZIP.
Installation starts disabled.
Enable it and restart the game.
Disable it and restart to restore original reads; then it can be updated or removed.
Profiles choose the next launch's content set.
Active content is deliberately immutable for the session.
Script-only Lua packages (no `content_dir`) still load live; a package with `content_dir` always needs a restart.

## Import layouts

The importer accepts:

- a simple `MyMod/content/...` tree
- a single-pack SDCafiine layout
- file-only Cemu packs with Definition metadata

Explicit SDCafiine and Cemu title IDs must include NSMBU USA `0005000010143500`.
ZIP wrappers are accepted if they contain exactly one content directory.
Multiple packs require selecting or extracting one pack first.

Known loose pack files (including `permanent_3d.pack` and all nine `permanent_2d_<Us|Eu|Jp><Language>.pack` language packs) can also be selected directly, or imported from a folder/ZIP with their original filenames.
They map to `Common/Pack/`.
Other loose files are placed where the installed game has a file of the same name, when it has exactly one (for example `Title_00.szs` goes to `Common/Layout/`).
Files the game does not have (read-me texts, pictures) are not used, and the package description lists what was placed and what was not.
A loose file whose name the game has several times, or an unknown `.pack`, requires an explicit content tree.

The importer writes a `kind: lua` package with `content_dir` set.
The source filename supplies a stable `content.<name>` ID, so same-named imports count as updates.
Supply an explicit manifest for a different ID, descriptive metadata, or version.

## Fan translations

A fan translation is usually a replaced 2D language pack (`permanent_2d_<Region><Language>.pack`: every message, the fonts, and the 2D layouts), often with a replaced title logo (`Common/Layout/Title_00.szs`).
Install it like any content mod: choose its folder or ZIP (loose files or a `content/` tree) and Install package, enable it, and restart.
The language pack applies to the language of the same name whatever region its file name has.
A translation shipped as `permanent_2d_EuEnglish.pack` (made for the European game) replaces English in the USA game.
A `permanent_2d_UsEnglish.pack` replaces English when English comes from a European language source ([../language-packs.md](../language-packs.md)).
A pack with the exact name the game asks for always comes first.
Choose the language the translation replaces (usually English) in Settings → Language.
No renaming is needed, and the game folder is never changed.
Arabic and Hebrew translations are shaped and laid out right to left by the port itself ([../rtl-text.md](../rtl-text.md)).
A Cemu code patch that such a translation ships for that purpose is not needed.
Install only its `content` folder: a package with a code patch is refused as a whole.

## What is rejected

Support covers **content file replacement only**.
The importer rejects code/meta/DLC folders, PPC patches (`patches.txt` and Cemu `.asm` code patches), shader files, and non-Definition Cemu rule sections.
A mod that ships content files together with a Cemu code patch is refused as a whole, so part of it is never dropped silently.
To use only its content files, select its `content` folder.
Randomizers and mixed code/data mods are not supported by this adapter.

These conventions follow the upstream [SDCafiine documentation](https://github.com/wiiu-env/sdcafiine_plugin) and [Cemu graphic-pack format](https://github.com/cemu-project/cemu_graphic_packs/wiki/How-to-create-Graphic-Packs).

## Manifest example

```json
{
  "format_version": 1,
  "id": "my-model",
  "name": "My local model replacement",
  "version": "1.0.0",
  "game_id": "nsmbu-usa",
  "minimum_manager_version": "1.0.0",
  "kind": "lua",
  "content_dir": "content"
}
```

No `lua.entry` or `main.lua` is required for a content-only package.
Keep the original game-relative filenames and directory structure under `content`.
The game must already be able to load the replacement format: archive/model sizes, joints, animations, and resource names must match the mod's target.
Raw PNG/DDS texture packs that expect Cemu's renderer interception are not equivalent to replacements of game archives and need a separate adapter.
A manifest marked compatible only means the host can load the package, not that every modified model or UI archive has been visually tested.

## Runtime behaviour

`runtime/src/mods/content.cpp` validates and indexes files at startup.
Enabled Lua packages that set `content_dir` feed the same immutable case-insensitive map.
Read-only FS opens, path stats, and read-only savestate handle reopens consult that map.
Writes, saves, code, and meta paths use the original resolver.
Missing paths fall through unchanged.
Directory enumeration retains original names but reports replacement sizes for replaced entries.
This adapter targets replacement of existing resources, not discovery of new files or deletion/hiding.
Conflicting enabled packages are rejected, rather than silently choosing a load order.
Content-only Lua packages cannot declare runtime options or dependencies unless they also ship a script entry that uses them.

Installed payloads should not be edited externally while the game runs.
Savestates must be used with the same active content set; savestate metadata does not record an asset fingerprint yet.
Import/staging limits are 4096 entries, 128 MiB per file, and 512 MiB per package.
Symlinks and path traversal are rejected.
Content files are copied into manager storage; the original game files are not changed, and game assets stay outside Git.

## Tests

`mod_content_startup`, `mod_content_fs_off`, and `mod_content_fs_on` cover startup activation and filesystem HLE against synthetic files.
See [mod-manager.md](mod-manager.md#validation).
