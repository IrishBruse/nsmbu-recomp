#!/usr/bin/env python3
import glob
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    disc = os.path.join(ROOT, "disc")
    images = sorted(glob.glob(os.path.join(disc, "*.wux")))
    if len(images) != 1:
        sys.exit("disc/ must contain one .wux file")
    image = images[0]
    common = None
    disc_key = None
    with open(os.path.join(disc, "keys.txt"), encoding="utf-8", errors="replace") as handle:
        for raw in handle:
            body, _, comment = raw.partition("#")
            hexkey = body.strip().lower()
            label = comment.strip()
            if len(hexkey) != 32:
                continue
            try:
                bytes.fromhex(hexkey)
            except ValueError:
                continue
            if label == "Common":
                common = hexkey
            elif label == "New Super Mario Bros U [USA, WUD]":
                disc_key = hexkey
    if common is None or disc_key is None:
        sys.exit("disc/keys.txt needs a Common key and a New Super Mario Bros U [USA, WUD] key")
    key_path = os.path.splitext(image)[0] + ".key"
    env = os.environ.copy()
    env["WIIU_COMMON_KEY"] = common
    try:
        with open(key_path, "w", encoding="ascii") as handle:
            handle.write(disc_key + "\n")
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
            env=env,
        )
    finally:
        if os.path.isfile(key_path):
            os.remove(key_path)


if __name__ == "__main__":
    main()
