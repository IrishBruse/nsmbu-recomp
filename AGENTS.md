## Play status

The first sentence under `## Status` in `README.md` is the current play status.
Update this sentence when play progress changes.
Include the release version in that sentence.

## Map

This repository is a fork of the Wind Waker HD recomp.

Read `docs/nsmbu.md` for the dump layout, the build, and the upstream base.
Read `game/README.md` for the files under `game/`.
Read the justfile for dev commands.

## Reproduce

Load a save state to reach 1-1.
Do this before you play through the title screen.

Slot 1 is `~/.config/nsmbu/states/slot1.bin`.
It is the start of 1-1.
`NSMBU_STATE_LOAD_AT` is `TV frame:slot`.
Frame 700 is after the boot threads exist.

```sh
mkdir -p .tmp/repro
NSMBU_NO_AUDIO=1 NSMBU_LOG_FILE=.tmp/repro/nsmbu.log \
NSMBU_STATE_LOAD_AT=700:1 NSMBU_EXIT_AT_FRAME=1100 \
./build/nsmbu --game game
```

The log line `Loaded slot 1` means the load worked.
A missing live thread logs `does not exist yet`.
Load at a later TV frame.
An exited thread in the save is ignored.
