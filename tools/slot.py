import json
import os
import re
import sys

_SLOT = re.compile(r"^([1-5])$")
_WORLD = re.compile(r"^\d+-\d+$")
LOAD_FRAME = 200
INTRO_SKIP_MOD = "example.intro-skip"


def take_args(args):
    slot = None
    pack = None
    rest = []
    for arg in args:
        if _WORLD.fullmatch(arg):
            if pack is not None:
                sys.exit("pass one course pack, such as 1-1")
            if slot is not None:
                sys.exit("pass a save slot or a course pack, not both")
            pack = arg
            continue
        match = _SLOT.fullmatch(arg)
        if match:
            if slot is not None:
                sys.exit("pass one save slot, such as 1")
            if pack is not None:
                sys.exit("pass a save slot or a course pack, not both")
            slot = int(match.group(1))
            continue
        rest.append(arg)
    return slot, pack, rest


def take_slot(args):
    slot, _, rest = take_args(args)
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


def apply_course_pack(user_dir, pack):
    path = os.path.join(user_dir, "mods.json")
    data = {}
    if os.path.isfile(path):
        with open(path, encoding="utf-8") as handle:
            data = json.load(handle)
    if not isinstance(data, dict):
        data = {}
    data.setdefault("format_version", 1)
    enabled = data.get("enabled")
    if not isinstance(enabled, dict):
        enabled = {}
    config = data.get("config")
    if not isinstance(config, dict):
        config = {}
    mod_cfg = config.get(INTRO_SKIP_MOD)
    if not isinstance(mod_cfg, dict):
        mod_cfg = {}
    enabled[INTRO_SKIP_MOD] = True
    mod_cfg["course_pack"] = pack
    config[INTRO_SKIP_MOD] = mod_cfg
    data["enabled"] = enabled
    data["config"] = config
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=2)
        handle.write("\n")
