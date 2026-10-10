#!/usr/bin/env python3
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, os.path.join(ROOT, "tools", "recomp"))
import builds
import game_dump


def check_update(game):
    usa = builds.canonical_build()
    update = game_dump.update_root(game)
    print("game: %s" % game)
    print("known build: %s  title %s  rpx sha256 %s" % (usa.name, usa.title_id, usa.sha256))
    print("required update folder: %s/" % game_dump.UPDATE_DIRNAME)
    print("")
    print("== update (USA 1.3.0) ==")
    errors, rpx = game_dump.folder_layout_errors(update, game_dump.UPDATE_DIRNAME)
    if rpx:
        e, _ = game_dump.check_rpx(
            rpx,
            "%s/code/red-pro2.rpx" % game_dump.UPDATE_DIRNAME,
            require_update_title=True,
        )
        errors.extend(e)
    if errors:
        print("")
        print("check failed:")
        for e in errors:
            print("  %s" % e)
        sys.exit(
            "put the USA 1.3.0 update under game/%s/ (code/, content/, meta/) before extract"
            % game_dump.UPDATE_DIRNAME
        )
    print("")
    print("update check ok")


def main():
    game = game_dump.game_root()
    image = os.path.join(game, "game.wux")
    key = os.path.join(game, "game.key")
    if not os.path.isfile(image):
        sys.exit("put your dump as game/game.wux")
    if not os.path.isfile(key):
        sys.exit("put the title key as game/game.key (next to game.wux)")

    check_update(game)

    print("")
    print("extracting base disc into game/...")
    subprocess.run(
        [
            sys.executable,
            os.path.join(ROOT, "tools", "wudextract.py"),
            image,
            "extract",
            game,
        ],
        check=True,
        cwd=ROOT,
    )

    print("applying game/%s onto game/..." % game_dump.UPDATE_DIRNAME)
    game_dump.apply_update(game)

    rpx = os.path.join(game, "code", "red-pro2.rpx")
    digest = builds.file_sha256(rpx)
    usa = builds.canonical_build()
    if digest != usa.sha256:
        sys.exit(
            "after applying the update, code/red-pro2.rpx is %s... (need %s...)"
            % (digest[:16], usa.sha256[:16])
        )
    print("extract ok: base disc + %s -> USA 1.3.0" % game_dump.UPDATE_DIRNAME)


if __name__ == "__main__":
    main()
