#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import devdata
import slot

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD = os.path.join(ROOT, "build")
LOAD_FRAME = 700
EXIT_FRAME = 1100
LOG = os.path.join(ROOT, ".tmp", "repro", "nsmbu.log")

PROFILE_ENV = {
    "NSMBU_NO_AUDIO": "1",
    "NSMBU_PROFILE": "1",
    "NSMBU_VK_STATS": "1",
    "NSMBU_LOG_SLOW_SWAP": "30",
}


def nsmbu_exe():
    for name in ("nsmbu", "nsmbu.exe"):
        path = os.path.join(BUILD, name)
        if os.path.isfile(path):
            return path
    return os.path.join(BUILD, "nsmbu")


def main():
    parser = argparse.ArgumentParser(description="Run a fixed-scene profile of NSMBU.")
    parser.add_argument("--exit-at", type=int, default=EXIT_FRAME, help=f"stop at this TV frame (default {EXIT_FRAME})")
    parser.add_argument("--load-at", type=int, default=LOAD_FRAME, help=f"load the save at this TV frame (default {LOAD_FRAME})")
    parser.add_argument("--log", default=LOG, help=f"log file path (default {LOG})")
    parser.add_argument("run_args", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    slot_num, run_args = slot.take_slot(args.run_args)
    if slot_num is None:
        slot_num = 1
    exe = nsmbu_exe()
    if not os.path.isfile(exe):
        sys.exit(f"missing {exe}; run: just build-release")
    game = os.path.join(ROOT, "game")
    if not os.path.isdir(game):
        sys.exit("missing game/; run: just extract")
    os.makedirs(os.path.dirname(args.log), exist_ok=True)
    env = os.environ.copy()
    _, save = devdata.apply_env(env, ROOT)
    for key, value in PROFILE_ENV.items():
        env.setdefault(key, value)
    env["NSMBU_LOG_FILE"] = args.log
    env["NSMBU_STATE_LOAD_AT"] = f"{args.load_at}:{slot_num}"
    env["NSMBU_EXIT_AT_FRAME"] = str(args.exit_at)
    path = slot.state_path(slot_num)
    if not os.path.isfile(path):
        sys.exit(f"missing {path}. Save a state in slot {slot_num}.")
    cmd = [exe, "--game", game, *devdata.with_save_arg(run_args, save)]
    print(f"[profile] log {args.log}", flush=True)
    print(f"[profile] load slot {slot_num} at TV frame {args.load_at}; exit at {args.exit_at}", flush=True)
    os.chdir(ROOT)
    raise SystemExit(subprocess.run(cmd, env=env).returncode)


if __name__ == "__main__":
    main()
