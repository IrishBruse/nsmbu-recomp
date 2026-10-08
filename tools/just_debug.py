#!/usr/bin/env python3
import os
import shutil

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

RUN_ENV_FILES = ("env-debug.txt", "env.txt")

DEFAULT_RUN_ENV = {
    "NSMBU_PROFILE": "1",
    "NSMBU_VK_STATS": "1",
    "NSMBU_SYNC_STATS": "1",
    "NSMBU_CRASH_RECOVERY": "1",
}


def parse_env_file(path, env):
    with open(path, encoding="utf-8", errors="replace") as handle:
        for raw in handle:
            line = raw.split("#", 1)[0].strip()
            if not line or "=" not in line:
                continue
            key, value = line.split("=", 1)
            key = key.strip()
            value = value.strip()
            if key:
                env[key] = value


def apply_run_debug(env, use_defaults=True):
    for name in RUN_ENV_FILES:
        path = os.path.join(ROOT, name)
        if os.path.isfile(path):
            parse_env_file(path, env)
    if use_defaults:
        for key, value in DEFAULT_RUN_ENV.items():
            if key not in env:
                env[key] = value


def debug_compile_flags(install_flags, sanitizer=False):
    flags = install_flags
    if sanitizer:
        flags = f"{flags} -fsanitize=address,undefined -fno-omit-frame-pointer"
    return flags


def debug_link_flags(link_dir, sanitizer=False):
    flags = f"-L{link_dir}"
    if sanitizer:
        flags = f"{flags} -fsanitize=address,undefined"
    return flags


def link_compile_commands(build_dir):
    src = os.path.join(build_dir, "compile_commands.json")
    dst = os.path.join(ROOT, "compile_commands.json")
    if os.path.isfile(src):
        if os.path.islink(dst) or os.path.isfile(dst):
            os.remove(dst)
        os.symlink(os.path.relpath(src, ROOT), dst)


def extra_cmake_debug(mods=False):
    if mods:
        return ["-DNSMBU_MODS_ENABLED=ON"]
    return []
