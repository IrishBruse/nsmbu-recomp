default:
    @just --list

extract:
    python3 tools/extract.py

build *args:
    python3 tools/build.py {{args}}

run *args:
    python3 tools/run.py {{args}}
