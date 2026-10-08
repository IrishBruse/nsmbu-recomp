#!/usr/bin/env python3
import argparse
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def nsmbu_exe():
    build = os.path.join(ROOT, "build")
    for name in ("nsmbu", "nsmbu.exe"):
        path = os.path.join(build, name)
        if os.path.isfile(path):
            return path
    return os.path.join(build, "nsmbu")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("run_args", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    exe = nsmbu_exe()
    if not os.path.isfile(exe):
        sys.exit(f"missing {exe}; run: just build")
    game = os.path.join(ROOT, "game")
    if not os.path.isdir(game):
        sys.exit("missing game/; run: just extract")
    cmd = [exe, "--game", game]
    cmd.extend(args.run_args)
    os.execv(exe, cmd)


if __name__ == "__main__":
    main()
