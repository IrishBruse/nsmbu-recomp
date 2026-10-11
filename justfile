# Local userdata for every recipe that runs the game (settings, states, save, mods).
# Portable installer releases keep using data/user via portable.txt next to the binary.
user_dir := justfile_directory() / "user"
export NSMBU_USER_DIR := user_dir

default:
    @just --list

extract:
    python3 tools/extract.py

build *args:
    python3 tools/build.py {{args}}

build-release *args:
    python3 tools/build.py --release {{args}}

build-sanitizer *args:
    python3 tools/build.py --sanitizer {{args}}

launch *args:
    python3 tools/launch.py {{args}}

launch-release *args:
    python3 tools/launch.py --no-debug {{args}}

launch-trace *args:
    python3 tools/launch.py --trace {{args}}

gdb *args:
    python3 tools/launch.py --gdb {{args}}

lldb *args:
    python3 tools/launch.py --lldb {{args}}

# Build, then launch. A number 1 through 5 loads that save slot.
run *args:
    just build {{args}} && just launch {{args}}

# Fixed-scene profile (slot 1 @ TV 700, exit 1100, log .tmp/repro/nsmbu.log). Use a Release build.
profile *args:
    python3 tools/profile.py {{args}}

# Type-check example Luau mods with the SDK definitions. Needs luau-lsp on PATH.
lint:
    find modding/examples -type f -name '*.luau' -print0 | xargs -0 luau-lsp analyze \
        --definitions:@nsmbu-core=modding/sdk/core.d.luau \
        --definitions:@nsmbu-guest=modding/sdk/guest.d.luau \
        --definitions:@nsmbu-hooks=modding/sdk/hooks.d.luau \
        --definitions:@nsmbu-input=modding/sdk/input.d.luau \
        --definitions:@nsmbu-data=modding/sdk/data.d.luau \
        --definitions:@nsmbu-level=modding/sdk/level.d.luau \
        --definitions:@nsmbu-player=modding/sdk/player.d.luau \
        --definitions:@nsmbu=modding/sdk/nsmbu.d.luau

recomp:
    python3 tools/recomp/recomp.py game/code/red-pro2.rpx build/gen
