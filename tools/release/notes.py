#!/usr/bin/env python3
"""Release notes: install instructions, one CHANGELOG.md version, and checksums.

usage: notes.py README.md VERSION SHA256SUMS.txt > notes.md

VERSION is the tag (v0.1) or the changelog heading (0.1).
The notes take the matching "## 0.1" block from CHANGELOG.md next to the README.
"""
import os
import re
import sys

def changelog_version(readme, version):
    path = os.path.join(os.path.dirname(os.path.abspath(readme)), "CHANGELOG.md")
    with open(path, encoding="utf-8") as f:
        text = f.read()
    ver = version[1:] if version.startswith("v") else version
    m = re.search(r"^## %s\s*$\n(.*?)(?=^## |\Z)" % re.escape(ver), text, re.S | re.M)
    return m.group(1).strip() if m else ""

def main():
    readme, version, sums = sys.argv[1:4]
    new = changelog_version(readme, version)
    with open(sums) as f:
        checksums = f.read().strip()
    print("""**New Super Mario Bros. U, native PC port, %s**

This release contains **no game files, no game code and no keys**. You need your own disc dump
(.wux/.wud with its disc key, plus the Wii U common key from your console), a Cemu .wua archive
(no keys needed), or an already extracted game folder. The installer builds the game from it on your machine.

**Install:** download the zip for your system, unzip it anywhere and start **NSMBU**. The
first start prepares the game once from your dump (about two minutes); later starts launch it directly.
Everything stays in that folder.
- macOS (Apple Silicon, macOS 14+): `NSMBU.app`. The release is not signed by Apple:
  macOS 15+: System Settings > Privacy & Security > Open Anyway; macOS 14: right-click > Open.
  Keep the app inside the unzipped folder (move the whole folder, not just the app)
- Windows (x86-64): `NSMBU.exe` (SmartScreen: "More info" > "Run anyway")
- Linux (glibc 2.35+, Vulkan): `nsmbu-launcher`; `linux-x86_64` for x86-64, `linux-aarch64` for arm64
  (Raspberry Pi 5, Asahi Linux, ARM laptops). Or one file: `chmod +x` the `.AppImage` and start it
  from anywhere (Steam Deck included); its game, code, saves and settings go to `~/.local/share/nsmbu`
  and `~/.config/nsmbu` instead of beside it (Ubuntu 24.04+: `libfuse2t64`, or
  `--appimage-extract-and-run`)

See "Install" in the README for details.

## What's new

%s

## Checksums (SHA-256)

```
%s
```
""" % (version, new or "See CHANGELOG.md.", checksums))

if __name__ == "__main__":
    main()
