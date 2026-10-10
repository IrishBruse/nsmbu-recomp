#!/usr/bin/env python3
import os
import re
import shutil
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools", "recomp"))
import builds

UPDATE_DIRNAME = "update"
UPDATE_TITLE_VERSION = 64


def game_root(path=None):
    return os.path.abspath(path or os.path.join(ROOT, "game"))


def update_root(game):
    return os.path.join(game, UPDATE_DIRNAME)


def read_title_version(folder):
    for rel, base in (("code/app.xml", 16), ("meta/meta.xml", 10)):
        path = os.path.join(folder, *rel.split("/"))
        try:
            with open(path, "rb") as f:
                data = f.read(65536)
        except OSError:
            continue
        tid = re.search(rb"<title_id[^>]*>\s*([0-9A-Fa-f]{16})\s*<", data)
        ver = re.search(rb"<title_version[^>]*>\s*([0-9A-Fa-f]+)\s*<", data)
        title = tid.group(1).decode().lower() if tid else None
        version = None
        if ver:
            try:
                version = int(ver.group(1), base)
            except ValueError:
                version = None
        if title is not None or version is not None:
            return title, version, rel
    return None, None, None


def title_matches_build(title, build):
    if not title:
        return True
    if title == build.title_id:
        return True
    if title == "0005000e" + build.title_id[8:]:
        return True
    return False


def folder_layout_errors(folder, label):
    errors = []
    rpx = os.path.join(folder, "code", "red-pro2.rpx")
    content = os.path.join(folder, "content")
    meta = os.path.join(folder, "meta", "meta.xml")
    if not os.path.isdir(folder):
        errors.append("missing folder: %s" % label)
        return errors, None
    if not os.path.isfile(rpx):
        errors.append("missing file: %s/code/red-pro2.rpx" % label)
    if not os.path.isdir(content):
        errors.append("missing folder: %s/content/" % label)
    if not os.path.isfile(meta):
        errors.append("missing file: %s/meta/meta.xml" % label)
    return errors, rpx if os.path.isfile(rpx) else None


def check_rpx(rpx_path, label, require_update_title=False):
    errors = []
    usa = builds.canonical_build()
    digest = builds.file_sha256(rpx_path)
    build = builds.by_sha256(digest)
    title, version, where = read_title_version(os.path.dirname(os.path.dirname(rpx_path)))
    print("%s: size %d  sha256 %s" % (label, os.path.getsize(rpx_path), digest))
    if title is not None or version is not None:
        print("  metadata (%s): title %s  version %s" % (
            where or "?",
            title or "?",
            version if version is not None else "?",
        ))
    if not build:
        errors.append(
            "%s is not the known USA 1.3.0 RPX (sha256 %s...; need %s...)"
            % (label, digest[:16], usa.sha256[:16])
        )
        return errors, None
    print("  match: %s (title %s)" % (build.name, build.title_id))
    if not title_matches_build(title, build):
        errors.append(
            "title id %s in %s does not match build %s (%s)"
            % (title, where, build.name, build.title_id)
        )
    if require_update_title:
        update_tid = "0005000e" + build.title_id[8:]
        if title and title != update_tid and title != build.title_id:
            errors.append(
                "%s title id must be %s or %s (got %s)"
                % (label, update_tid, build.title_id, title)
            )
        if version is not None and version != UPDATE_TITLE_VERSION:
            errors.append(
                "%s title version must be %d (got %s)"
                % (label, UPDATE_TITLE_VERSION, version)
            )
    return errors, build


def apply_update(game):
    src = update_root(game)
    for name in ("code", "content", "meta"):
        src_part = os.path.join(src, name)
        dst_part = os.path.join(game, name)
        if not os.path.isdir(src_part):
            raise SystemExit("update is incomplete: missing %s/%s" % (UPDATE_DIRNAME, name))
        os.makedirs(dst_part, exist_ok=True)
        shutil.copytree(src_part, dst_part, dirs_exist_ok=True)
