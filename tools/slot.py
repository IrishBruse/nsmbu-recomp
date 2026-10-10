import os
import re
import sys

_SLOT = re.compile(r"^([1-5])$")
_WORLD = re.compile(r"^\d+-\d+$")
LOAD_FRAME = 200


def take_slot(args):
    slot = None
    rest = []
    for arg in args:
        if _WORLD.fullmatch(arg):
            sys.exit(f"{arg} is not a save slot. Use the slot number, such as 1.")
        match = _SLOT.fullmatch(arg)
        if match:
            if slot is not None:
                sys.exit("pass one save slot, such as 1")
            slot = int(match.group(1))
            continue
        rest.append(arg)
    return slot, rest


def state_path(slot):
    override = os.environ.get("NSMBU_STATE_DIR")
    if override:
        directory = override
    elif os.environ.get("NSMBU_USER_DIR"):
        directory = os.path.join(os.environ["NSMBU_USER_DIR"], "states")
    elif sys.platform == "darwin":
        directory = os.path.join(os.path.expanduser("~"), "Library", "Application Support", "nsmbu", "states")
    elif sys.platform == "win32":
        directory = os.path.join(os.environ.get("APPDATA") or ".", "NSMBU", "states")
    else:
        root = os.environ.get("XDG_CONFIG_HOME") or os.path.join(os.path.expanduser("~"), ".config")
        directory = os.path.join(root, "nsmbu", "states")
    return os.path.join(directory, f"slot{slot}.bin")


def apply_slot(env, slot):
    path = state_path(slot)
    if not os.path.isfile(path):
        sys.exit(f"missing {path}. Save a state in slot {slot}.")
    env.setdefault("NSMBU_STATE_LOAD_AT", f"{LOAD_FRAME}:{slot}")
