# New Super Mario Bros. U — native port

This is a native port of the Wii U game.
The port supports the USA version only.
The installer translates the PowerPC code to C on your computer.
Cafe OS and GX2 run as native code.
Linux and Windows use Vulkan.
macOS uses Metal.

You supply your own copy of the game.
A release contains the runtime, the tools, and the installer.
You supply the disc image, the keys, and the game files.

## Status

In 0.2.0 the World 1-1 crash is fixed; World 1-1 through 1-3 are playable with low performance.

## Install

Download the zip for your system from the [Releases](https://github.com/IrishBruse/nsmbu-recomp/releases) page.
Unzip it into a folder that you can write to.
Start **NSMBU**.

| System | File to start |
|--------|----------------|
| macOS, Apple Silicon, macOS 14 or newer | `NSMBU.app` |
| Windows, x86-64, Windows 10 or 11, Vulkan | `NSMBU.exe` |
| Linux, x86-64, glibc 2.35 or newer, Vulkan | `nsmbu-launcher` in the `linux-x86_64` zip |
| Linux, arm64, glibc 2.35 or newer, Vulkan | `nsmbu-launcher` in the `linux-aarch64` zip |

The first start opens the installer.
Each later start launches the game.

The installer asks which copy of the game you have.
Choose one option.

1. A disc image (`.wux` or `.wud`), its keys, and the USA **1.3.0** update folder.
2. A Cemu archive (`.wua`) that already has title version 64.
   This file needs no keys.
3. An extracted game folder that already has the 1.3.0 update applied.
   The folder contains `code`, `content`, and `meta`.

The installer then opens a file window and asks you to choose that file or folder.
macOS uses the system file chooser.
Windows uses the system file chooser.
Linux uses Zenity or KDialog when one of those programs is installed.

When no file window opens, the installer asks you to type the path.
You can drag the file into the window and press Enter.

A disc image also needs a disc key, the Wii U common key, and the USA 1.3.0 update.
Put the update in an `update/` folder next to the disc image (`code/`, `content/`, `meta/`), or choose that folder in the installer.
The installer uses a `.key` file that has the same name as the image when that file is next to the image.
The installer uses `common.key` when that file is next to the image.
When a key is missing, the installer asks you to choose a key file or to paste 32 hex digits.
The paste field hides the text.
The keys stay in memory until extraction starts.
The setup log contains no keys.

The installer accepts USA New Super Mario Bros. U, title `00050000-10101d00`, title version 64 (1.3.0).
For a disc image it checks the update folder, extracts the disc, applies the update, then checks `code/red-pro2.rpx` before it translates the code.

The installer then prepares the game.
For a disc image or a Cemu archive, it extracts the files into the release folder.
An extracted folder stays in its original location.
The installer translates the code to C, compiles it, and links it with the runtime.

The first start takes a few minutes.
Start **NSMBU** again to play.

To repair, update, or choose a different game file, open the installer again.
On macOS or Windows, hold Shift while you start **NSMBU**.
On Linux, start `nsmbu-launcher --setup`, or use the Setup action of `NSMBU.desktop`.

The release folder is portable.
The built game, the game files, the saves, and the settings stay in `data/` inside that folder.

On macOS the app is unsigned.
On macOS 14, right-click `NSMBU.app`, choose **Open**, then choose **Open**.
On macOS 15 and newer, open **System Settings**, then **Privacy & Security**, then **Open Anyway**.
Keep `NSMBU.app` inside the unzipped folder.
Move the whole folder when you want the game in another place.

On Windows, SmartScreen can block an unsigned program.
Choose **More info**, then **Run anyway**.

The terminal installer is the fallback when the window cannot run.
On macOS, run `tools/Setup in Terminal.command`.
On Windows, run `tools/Setup in a console window.bat`.
On Linux, run `tools/setup-in-terminal.sh`.
The terminal installer asks the same questions and opens the same file window when the system provides one.

## Controls

Press **F1** to open the settings overlay.
Open **Controls** to change keys and controller buttons.

A release stores the mapping in `data/user/controls.json`.
A build from source stores it in the port config directory.
That directory is `~/.config/nsmbu` on Linux, `%APPDATA%\NSMBU` on Windows, and `~/Library/Application Support/NSMBU` on macOS.
Set `NSMBU_CONTROLS` to use another file.

Host gamepads use the same layout as the Wii U GamePad.
The bottom face button (Xbox **A**) is Wii U **B**.
The right face button (Xbox **B**) is Wii U **A**.

Default keyboard layout:

| Keyboard | Wii U input |
|----------|-------------|
| W A S D | left stick |
| arrow keys | right stick (camera) |
| K or Space | A |
| J | B |
| L | X |
| Left Shift | Y |
| Q / E | L / R |
| Left Control | ZR |
| Enter / Tab | + / − |
| H | Home |
| 1 2 3 4 | D-pad up / down / left / right |
| X / V | left / right stick click |

Gyro aiming and other motion options are in **Controls → Gyro…**.
See [docs/gyro.md](docs/gyro.md).

## Developers

The last merge from [ZeldaWWHDRecomp](https://github.com/ZeldaWWHDRecomp/ZeldaWWHDRecomp) is commit `defb89f21607345e2e97b2c145a071a3f362640c`.

These commands are for a source checkout.
Players use a [release](#install).

Install `just`, CMake 3.20 or newer, Python 3, and Clang.

Put your dump in `game/`.
The file layout is in [game/README.md](game/README.md).

| Command | Result |
|---------|--------|
| `just` | List the commands |
| `just extract` | Check `game/update/` (USA 1.3.0), extract `game/game.wux`, then apply the update |
| `just build` | Configure `build/` in Debug and build `nsmbu` |
| `just build-release` | Build in Release |
| `just build-sanitizer` | Build with AddressSanitizer and UBSan |
| `just launch` | Start `build/nsmbu` with `--game` set to `game/` |
| `just launch-release` | Start without the debug environment defaults |
| `just launch-trace` | Start with `--trace` for HLE logging |
| `just run` | Run `just build`, then `just launch` |
| `just run 3` | Same, and load save slot 3 |
| `just recomp` | Translate `game/code/red-pro2.rpx` into `build/gen` |
| `just gdb` | Start the game under gdb |
| `just lldb` | Start the game under lldb |

When `game/code/red-pro2.rpx` is newer than the generated code, `just build` runs the recompiler.
When that file is absent, the build uses stubs so the tree can link.

`just launch` sets `NSMBU_PROFILE`, `NSMBU_VK_STATS`, `NSMBU_SYNC_STATS`, and `NSMBU_CRASH_RECOVERY` when those variables are unset.

Build notes and the upstream base are in [docs/nsmbu.md](docs/nsmbu.md).
The archived Wind Waker HD readme is in [docs/upstream-wwhd-readme.md](docs/upstream-wwhd-readme.md).
Changes are in [CHANGELOG.md](CHANGELOG.md).
