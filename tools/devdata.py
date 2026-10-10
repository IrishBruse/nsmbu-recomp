import json
import os
import shutil

USER_NAME = "user"


def paths(root):
    user = os.path.join(root, USER_NAME)
    save = os.path.join(user, "save")
    return user, save


def ensure(root):
    user, save = paths(root)
    os.makedirs(user, exist_ok=True)
    old_save = os.path.join(root, "save")
    if os.path.isdir(old_save) and not os.path.exists(save):
        shutil.move(old_save, save)
    else:
        os.makedirs(save, exist_ok=True)
    return user, save


def sync_example_mods(root):
    if os.environ.get("NSMBU_MOD_MANAGER_DIR"):
        return
    if os.environ.get("NSMBU_NO_EXAMPLE_MODS") == "1":
        return
    src_root = os.path.join(root, "modding", "examples")
    if not os.path.isdir(src_root):
        return
    mods_dir = os.path.join(root, USER_NAME, "ModManager", "Mods")
    os.makedirs(mods_dir, exist_ok=True)
    for name in sorted(os.listdir(src_root)):
        src = os.path.join(src_root, name)
        manifest_path = os.path.join(src, "manifest.json")
        if not os.path.isdir(src) or not os.path.isfile(manifest_path):
            continue
        with open(manifest_path, encoding="utf-8") as f:
            package_id = json.load(f).get("id")
        if not isinstance(package_id, str) or not package_id:
            continue
        dest = os.path.join(mods_dir, package_id)
        if os.path.isdir(dest):
            shutil.rmtree(dest)
        shutil.copytree(src, dest)


def apply_env(env, root):
    user, save = ensure(root)
    env.setdefault("NSMBU_USER_DIR", user)
    os.environ.setdefault("NSMBU_USER_DIR", env["NSMBU_USER_DIR"])
    sync_example_mods(root)
    return user, save


def has_save_arg(args):
    for arg in args:
        if arg == "--save" or arg.startswith("--save="):
            return True
    return False


def with_save_arg(args, save):
    if has_save_arg(args):
        return list(args)
    return ["--save", save, *args]
