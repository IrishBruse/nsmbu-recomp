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

Put your `.wux` dump as `game.wux` and its title key as `game.key` in the repository root.

Put the Wii U common key in `common.key` in the repository root, or set `WIIU_COMMON_KEY`.

Git ignores `game.wux`, `game.key`, `common.key`, and the extracted tree in `game/`.

Run `just extract`.

That command reads `game.wux` and writes `game/code/`, `game/content/`, and `game/meta/`.
