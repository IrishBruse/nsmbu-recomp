# Local userdata for every recipe that runs the game (settings, states, save, mods).
# Portable installer releases keep using data/nsmbu via portable.txt next to the binary.
nsmbu_dir := justfile_directory() / "nsmbu"
export NSMBU_USER_DIR := nsmbu_dir

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

# Release build, then a launch with no debug defaults, like the released install.
run-release *args:
    just build-release {{args}} && just launch-release {{args}}

# Ninja Release build under build/rel, package a real SDK, then open setup.
installer *args:
    #!/usr/bin/env bash
    set -euo pipefail
    rel=build/rel
    cmake_extra=(-G Ninja -DNSMBU_SETUP_GUI=ON)
    python3 tools/build.py --release --dir "$rel" --target nsmbu -- "${cmake_extra[@]}" {{args}}
    python3 tools/build.py --release --dir "$rel" --target nsmbu-extract -- "${cmake_extra[@]}" {{args}}
    python3 tools/build.py --release --dir "$rel" --target nsmbu-setup -- "${cmake_extra[@]}" {{args}}
    export NSMBU_INSTALLER_BUILD="$PWD/$rel"
    pkg=$(python3 tools/installer/dev_pkg.py)
    export NSMBU_SETUP_PKG="$pkg/"
    if [[ -x "$pkg/nsmbu-launcher" ]]; then
      exec "$pkg/nsmbu-launcher" --setup
    fi
    exec "$rel/nsmbu-setup" --setup

# Fixed-scene profile (slot 1 @ TV 700, exit 1100, log .tmp/repro/nsmbu.log). Use a Release build.
profile *args:
    python3 tools/profile.py {{args}}

# perf CPU profile of the fixed 1-1 scene into .tmp/perf/ (attach: just perf --attach PID --seconds 30)
perf *args:
    python3 tools/perf.py {{args}}

# Type-check example Luau mods with the SDK definitions. Needs luau-lsp on PATH.
lint:
    find modding/examples -type f -name '*.luau' -print0 | xargs -0 luau-lsp analyze \
        --definitions:@nsmbu=modding/sdk/nsmbu.d.luau

recomp:
    python3 tools/recomp/recomp.py game/code/red-pro2.rpx build/gen
