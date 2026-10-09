# `game/` — your copy of NSMBU

This folder holds **your** Wii U dump, keys, and extracted game files.

Nothing here except this readme is in git.

## Before `just extract`

Put these files in **this** folder (`game/`):

| File | What it is |
|------|------------|
| `game.wux` | Your USA New Super Mario Bros. U disc image (`.wux`) |
| `game.key` | Title key for that image (16 raw bytes or one line of 32 hex digits) |
| `common.key` | Wii U common key from your console (same formats), **or** set `WIIU_COMMON_KEY` in the environment |

`game.key` must be the key for **this** `game.wux` (dumped with the image).

## After `just extract`

From the repository root, run:

```bash
just extract
```

That reads `game/game.wux` and writes:

| Path | Contents |
|------|----------|
| `code/` | Executables (for example `red-pro2.rpx`) |
| `content/` | Game assets |
| `meta/` | Title metadata |

The port runs with `--game` pointing at this folder.

## More detail

See the root [README.md](../README.md) and [docs/nsmbu.md](../docs/nsmbu.md).
