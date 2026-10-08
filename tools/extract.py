#!/usr/bin/env python3
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    image = os.path.join(ROOT, "game.wux")
    key = os.path.join(ROOT, "game.key")
    if not os.path.isfile(image):
        sys.exit("put your dump as game.wux in the repo root")
    if not os.path.isfile(key):
        sys.exit("put the title key as game.key in the repo root (next to game.wux)")
    subprocess.run(
        [
            sys.executable,
            os.path.join(ROOT, "tools", "wudextract.py"),
            image,
            "extract",
            os.path.join(ROOT, "game"),
        ],
        check=True,
        cwd=ROOT,
    )


if __name__ == "__main__":
    main()
