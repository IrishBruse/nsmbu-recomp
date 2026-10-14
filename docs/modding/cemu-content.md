# Plan: native Cemu content packs (levels and file replacements)

Status: **proposal (not implemented)**.
This plan covers large Cemu file-only packs that replace game content (levels, actors, layouts, sound).
An example is a pack such as Cloudy SMBU 2: a pack root with `rules.txt`, a `content/` tree, and optional Cemu `meta/` title art.

The Mods tab has no import UI.
Players copy package folders into the mods directory ([mod-manager.md](mod-manager.md)).
Today a Cemu level pack still needs a hand-written `manifest.json`, and size caps reject full soundtracks.
See the live behaviour in [content.md](content.md) and graphics-only Cemu packs in [cemu.md](cemu.md).

## Goal

A player opens **Mods → Open mods folder** and drops the **pack root** into `mods/` (or a subfolder whose name becomes the package id).
On scan, the port detects a file-only Cemu content pack from `rules.txt` and the `content/` tree.
The player does **not** supply a `manifest.json` (or any other metadata JSON).
The scanner writes or updates `manifest.json` beside the pack and fills id, name, and version from `rules.txt` when present.
After enable and restart, reads under `/vol/content/` use the replacements.

Graphics and shader packs stay on `kind: cemu` ([cemu.md](cemu.md)).
Scripted mods stay on `kind: luau` ([luau-mods.md](luau-mods.md)).

## Why the current path is not enough

| Gap | Current behaviour | Needed for full level packs |
| --- | --- | --- |
| Pack root | A sibling `meta/` folder fails content tooling; scan requires `manifest.json` | Ignore Cemu title `meta/`; accept Definition-only `rules.txt` + `content/` on scan |
| Manifest | Player must ship or invent `manifest.json` | Synthesize from `rules.txt` `[Definition]` when that file is present |
| Size | 128 MiB per file and 512 MiB per package (`content.cpp`, `mod_archive.cpp`) | Raise caps so a full soundtrack and level set fit (for example a ~236 MiB `.bfsar` and a pack over 1 GiB) |
| Kind routing | Graphics packs need an explicit `kind: cemu` manifest | Definition-only + `content/` → content/`luau`; Texture/Control/Preset → graphics/`cemu` |

## Detection (no player JSON)

When scanning `mods/<entry>/`, classify as follows.

1. If the tree has a valid `manifest.json` with `kind` / `content_dir` / `luau` / `cemu`, keep the existing package path.
2. Else if `rules.txt` has graphics sections (`[Texture…]`, `[Control…]`, `[Preset…]`, shaders, and so on), treat it as `kind: cemu` and synthesize the graphics manifest ([cemu.md](cemu.md)).
3. Else if `rules.txt` is Definition-only (or Definition plus ignored non-load sections) **and** exactly one `content/` tree exists under the pack, treat it as a **Cemu content pack**.
4. Else leave the folder as a broken package with a clear problem string (same as a missing manifest today).

For step 3 the host writes `manifest.json` into that mods folder entry.
The player did not author that file.

Suggested fields from `[Definition]`:

| `rules.txt` | Generated manifest |
| --- | --- |
| `name` | `name` |
| `version` (Cemu pack version integer or string) | `version` as three-part semver when possible; else `1.0.0` with the raw value in `description` |
| `titleIds` | Must include USA NSMBU `0005000010101d00` (same check as today) |
| (none) | `id` = slug from `name`, or the folder name when it is already a valid id |
| (none) | `kind` = `luau`, `content_dir` = relative path to the detected `content/` |
| `description` | Optional `description` |

Do not require the player to edit or ship JSON.
An optional player-supplied `manifest.json` in the pack may still override id or display fields when present and valid.

## Layout rules for Cemu content packs

Accepted pack under the mods folder (example):

```text
mods/cloudy-smbu-2/
  rules.txt          # [Definition] only; titleIds includes 0005000010101d00
  content/           # game-relative replacements
  meta/              # ignored (boot/icon TGA, meta.xml); not mapped into /vol/content
  manifest.json      # written by scan when missing
```

Rules:

- Map only files under `content/` into the replacement index (same engine as today).
- Ignore `meta/` for content packs; do not map it into `/vol/content`.
- Reject `code/`, non-empty patch payloads (`.asm`, `patches.txt`), shader files, and mixed graphics+content in one folder (same refusal policy as [content.md](content.md): do not drop half the pack silently).
- Title ID checks stay USA 1.3.0 only.
- The folder name may differ from the package `id`.

## Size limits

Raise the shared caps used by content indexing and archive staging so full level mods fit.

| Limit | Live today | Proposed floor |
| --- | --- | --- |
| Per file | 128 MiB | **512 MiB** (covers large `cafe_redpro_sound.bfsar`-class archives) |
| Per package (total) | 512 MiB | **2 GiB** (covers multi-world level sets with OST and demos) |
| Entry count | 4096 | Keep **4096** unless real packs exceed it; raise only with evidence |

Apply the same numbers in:

- `runtime/src/mods/content.cpp` (`index`)
- `runtime/src/mods/mod_archive.cpp` (staging / expand)
- any package copy check that still uses 128 MiB (for example oversized replacement pack checks in `packages.cpp`)

Document the new caps in [content.md](content.md) when the code lands.
Keep rejecting symlinks and path traversal.

Files already live under `mods/` after the player copies them.
No second install copy is required for this path.

## Runtime and UI

- Keep using the existing immutable content map and restart-on-enable rules ([content.md](content.md)).
- **Open mods folder** stays the only install action in the Mods tab.
- Show the generated name and version from `rules.txt` in the Mods tab.
- Surface a clear error when title IDs miss USA NSMBU, when graphics rules are mixed with `content/`, or when size/entry caps still fail.
- Profiles and disable/update behaviour stay the same as other `content_dir` packages.

## Out of scope

- Applying Cemu PPC / `.asm` code patches or randomizers.
- Mapping `meta/` into the host window icon or boot splash (optional later).
- Folding graphics packs into Luau.
- Guaranteeing every fan level archive loads; the host only replaces files the game already opens.
- Bringing back an in-game file picker or ZIP import UI.

## Implementation sketch

1. **Classify** during `scan()`: Definition-only `rules.txt` + `content/` → synthesize content/`luau` manifest; ignore sibling `meta/`.
2. **Synthesize** `manifest.json` from `[Definition]` (no player JSON required); folder name may differ from `id`.
3. **Raise** per-file and per-package limits to the proposed floor in content and archive code paths.
4. **Tests**: synthetic mods-folder pack with `rules.txt` + `content/` + `meta/`; title-id failure; oversized file under the new cap that would have failed at 128 MiB; reject Definition+Preset+`content/` mix.
5. **Docs**: mark this page live or merge the player steps into [content.md](content.md); update [cemu.md](cemu.md) to point content packs here.

## Validation target

A pack like Cloudy SMBU 2 is copied into the mods folder as its **root**, appears after scan without a hand-written manifest, enables, restarts, and replaces course / layout / sound reads.
The soundtrack file and full `content/` tree stay under the new size caps.
