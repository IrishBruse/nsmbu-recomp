# New Super Mario Bros. U — native port

This tree starts from [NSMBURecomp](https://github.com/NSMBURecomp/NSMBURecomp) commit `853d7b18c8c6703c5fc50c40cb923c1fb9503ecd`.

Use that commit as the rebase base.

Details are in [docs/nsmbu.md](docs/nsmbu.md).

NSMBU is a Wii U game.

It uses Cafe OS and GX2.

The recompiler and the native runtime stay.

The upstream Wind Waker HD launcher names do not stay.

The upstream readme is in [docs/upstream-nsmbu-readme.md](docs/upstream-nsmbu-readme.md).

Port notes are in [docs/nsmbu.md](docs/nsmbu.md).

Game files stay on your machine.

This repository does not contain a disc image, a title key, or recompiled game code.

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
