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

## Disc image

Put your disc image and `keys.txt` in `disc/`.

Git ignores everything in that folder except `disc/.gitkeep`.

Run `just extract`.

That command reads the `.wux` and `keys.txt` in `disc/`.

It writes the game into `game/`.
