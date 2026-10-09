#!/usr/bin/env python3
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def main():
    game_dir = os.path.join(ROOT, "game")
    image = os.path.join(game_dir, "game.wux")
    key = os.path.join(game_dir, "game.key")
    if not os.path.isfile(image):
        sys.exit("put your dump as game/game.wux")
    if not os.path.isfile(key):
        sys.exit("put the title key as game/game.key (next to game.wux)")
    subprocess.run(
        [
            sys.executable,
            os.path.join(ROOT, "tools", "wudextract.py"),
            image,
            "extract",
            game_dir,
        ],
        check=True,
        cwd=ROOT,
    )

if __name__ == "__main__":
    main()
