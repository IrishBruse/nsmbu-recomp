---
name: changelog
description: Use when editing CHANGELOG.md
---

# Changelog summary (players)

Write a short summary for people who run the **release / launcher** build.
The audience is players, not developers.

## Steps

1. Read `CHANGELOG.md`.
   Use **Unreleased** unless the user names a version heading.
2. Keep only **player-facing** items (see Include).
   Drop every **developer** item (see Drop).
3. Rewrite each kept item in plain player language.
   Use **release paths** for folders (`data/nsmbu/...`).
   Prefer **Mods → Open mods folder** when that is how the player finds a path.
4. Order: lead with the largest player feature, then other play/UI changes, then fixes.
5. Output short sections or bullets.
   One idea per sentence.

Done when every bullet is something a launcher user can notice, do, or benefit from, and no developer tooling, repo paths, or API internals remain.

## Include

- New or changed player features (mods, overlay, audio, controls, display, saves, performance the player feels)
- Where to put files in a **release** build (`data/nsmbu/mods/`, `data/nsmbu/save/`, and so on)
- Overlay or Mods tab UI the player sees
- Playable progress, crash fixes, and gameplay fixes
- Installer / first-start behaviour the player runs

## Drop

- `just`, CMake, recompiler, stubgen, headers, `references/`, `tools/`, tests, CI
- Example mod packages, Luau/SDK/API surface, hook names, guest addresses
- Source-tree userdata (`nsmbu/`), symlink recipes, env vars for developers
- Internal docs moves (`docs/…`) unless the player must open that doc
- Implementation detail that does not change what the player does

## Path language

| Say | Do not say |
| --- | --- |
| `data/nsmbu/mods/` (or **Open mods folder**) | `nsmbu/mods/`, `<host config>/mods/`, `modding/examples/` |
| `data/nsmbu/save/` | `--save`, `NSMBU_USER_DIR` |
| settings / controls in the release data folder | `settings.ini` path recipes for source builds |

AppImage / non-portable builds may use the platform config folder instead of `data/nsmbu/`.
Name that only when the user asks about AppImage or non-portable installs.

## Shape

```markdown
**Biggest player feature**

One or two sentences: what it is, where it lives in the release, what to do (enable, restart).

**Other changes**

- …
- …

**Fixes**

- …
```

Lead heading and bullet count may shrink when Unreleased is small.
Do not invent items that are not in the changelog.
