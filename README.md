# New Super Mario Bros. U — native port

This tree starts from [NSMBURecomp](https://github.com/NSMBURecomp/NSMBURecomp) commit `853d7b18c8c6703c5fc50c40cb923c1fb9503ecd`.

NSMBU is a Wii U game on Cafe OS and GX2.

The recompiler and native runtime stay.

Wind Waker HD launcher names do not stay.

This repository does not contain a disc image, a title key, or recompiled game code.

Game files stay on your machine.

## Game dump

Keep everything in `game/`:

| File | Purpose |
|------|---------|
| `game/game.wux` | Your `.wux` disc image |
| `game/game.key` | Title key for that image (16 bytes or 32 hex digits) |
| `game/common.key` | Wii U common key from your console (or set `WIIU_COMMON_KEY`) |
| `game/code/`, `game/content/`, `game/meta/` | Filled by `just extract` |

`.gitignore` ignores `game/` except [game/README.md](game/README.md), which describes this layout.

Run `just extract`.

That command reads `game/game.wux` and writes the `code/`, `content/`, and `meta/` folders under `game/`.

## Documentation

Use commit `853d7b18c8c6703c5fc50c40cb923c1fb9503ecd` as the rebase base.

Port notes, build steps, and rebase commands are in [docs/nsmbu.md](docs/nsmbu.md).

The upstream readme is in [docs/upstream-nsmbu-readme.md](docs/upstream-nsmbu-readme.md).
