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

run *args:
    just build {{args}} && just launch {{args}}

recomp:
    python3 tools/recomp/recomp.py game/code/red-pro2.rpx build/gen
