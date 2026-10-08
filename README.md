# New Super Mario Bros. U — native port

This tree starts from [ZeldaWWHDRecomp](https://github.com/ZeldaWWHDRecomp/ZeldaWWHDRecomp) commit `853d7b18c8c6703c5fc50c40cb923c1fb9503ecd`.

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

## Controls

Press **F1** for the settings overlay.

Open **Controls** to remap keys and controller buttons.

The mapping is saved to `controls.json` under the port config directory (`~/.config/nsmbu` on Linux, `%APPDATA%\NSMBU` on Windows, `~/Library/Application Support/NSMBU` on macOS).

Set `NSMBU_CONTROLS` to use another file.

Host gamepads use positional mapping: the bottom face button (Xbox **A**) is Wii U **B**, the right face button (Xbox **B**) is Wii U **A**, and the other buttons follow the same layout.

Default keyboard layout:

| Keyboard | Wii U input |
|----------|-------------|
| W A S D | left stick |
| arrow keys | right stick (camera) |
| K or Space | A |
| J | B |
| L | X |
| I | Y |
| Q / E | L / R |
| Left Shift | ZL |
| C | ZR |
| Enter / Tab | + / − |
| H | Home |
| 1 2 3 4 | D-pad up / down / left / right |
| X / V | left / right stick click |

Gyro aiming and other motion options are in **Controls → Gyro…** ([docs/gyro.md](docs/gyro.md)).

## Documentation

Use commit `853d7b18c8c6703c5fc50c40cb923c1fb9503ecd` as the rebase base.

Port notes, build steps, and rebase commands are in [docs/nsmbu.md](docs/nsmbu.md).

The upstream readme is in [docs/upstream-nsmbu-readme.md](docs/upstream-nsmbu-readme.md).
