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

run *args:
    python3 tools/run.py {{args}}

run-release *args:
    python3 tools/run.py --no-debug {{args}}

run-trace *args:
    python3 tools/run.py --trace {{args}}

gdb *args:
    python3 tools/run.py --gdb {{args}}

lldb *args:
    python3 tools/run.py --lldb {{args}}

debug *args:
    just build {{args}} && just run {{args}}

recomp:
    python3 tools/recomp/recomp.py game/code/red-pro2.rpx build/gen
