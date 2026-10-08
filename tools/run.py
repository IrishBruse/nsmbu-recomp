#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys

from just_debug import ROOT, apply_run_debug

BUILD = os.path.join(ROOT, "build")


def nsmbu_exe():
    for name in ("nsmbu", "nsmbu.exe"):
        path = os.path.join(BUILD, name)
        if os.path.isfile(path):
            return path
    return os.path.join(BUILD, "nsmbu")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--no-debug", action="store_true")
    parser.add_argument("--gdb", action="store_true")
    parser.add_argument("--lldb", action="store_true")
    parser.add_argument("--trace", action="store_true", help="pass --trace to the game (HLE trace)")
    parser.add_argument("run_args", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    use_debug_env = not args.no_debug
    exe = nsmbu_exe()
    if not os.path.isfile(exe):
        sys.exit(f"missing {exe}; run: just build")
    game = os.path.join(ROOT, "game")
    if not os.path.isdir(game):
        sys.exit("missing game/; run: just extract")
    env = os.environ.copy()
    if use_debug_env:
        apply_run_debug(env)
    cmd = [exe, "--game", game]
    if args.trace:
        cmd.append("--trace")
    cmd.extend(args.run_args)
    os.chdir(ROOT)
    if args.gdb:
        gdb = shutil.which("gdb")
        if not gdb:
            sys.exit("gdb not found on PATH")
        raise SystemExit(subprocess.run([gdb, "--args", *cmd], env=env).returncode)
    if args.lldb:
        lldb = shutil.which("lldb")
        if not lldb:
            sys.exit("lldb not found on PATH")
        raise SystemExit(subprocess.run([lldb, "--", *cmd], env=env).returncode)
    os.execve(exe, cmd, env)


if __name__ == "__main__":
    main()
