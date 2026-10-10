# `game/` — your copy of NSMBU

This folder holds **your** Wii U dump, keys, update, and extracted game files.

Nothing here except this readme is in git.

## Before `just extract`

Put these in **this** folder (`game/`):

| Path | What it is |
|------|------------|
| `game.wux` | Your USA New Super Mario Bros. U disc image (`.wux`) |
| `game.key` | Title key for that image (16 raw bytes or one line of 32 hex digits) |
| `common.key` | Wii U common key from your console (same formats), **or** set `WIIU_COMMON_KEY` |
| `update/` | The USA **1.3.0** update (title version 64), already extracted |

### `update/` layout

| Path | Contents |
|------|----------|
| `update/code/red-pro2.rpx` | Update executable (must match the known USA 1.3.0 hash) |
| `update/content/` | Update content |
| `update/meta/` | Update metadata |

`game.key` must be the key for **this** `game.wux`.

## Extract

From the repository root:

```bash
just extract
```

That:

1. Checks `game/update/` is USA 1.3.0.
2. Extracts `game/game.wux` into `game/code`, `game/content`, `game/meta`.
3. Applies `game/update/` over those folders and checks the merged RPX.

The port runs with `--game` pointing at this folder.

## More detail

See the root [README.md](../README.md) and [docs/nsmbu.md](../docs/nsmbu.md).
