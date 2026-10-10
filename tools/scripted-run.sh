#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
name=${1:-survey}
if [ -z "${NSMBU_EXIT_AT_FRAME:-}" ]; then
    echo "Set NSMBU_EXIT_AT_FRAME" >&2
    exit 2
fi
user="$root/user"
save_src="$user/save"
if [ ! -d "$save_src" ] && [ -d "$root/save" ]; then
    save_src="$root/save"
fi
if [ ! -d "$save_src" ]; then
    echo "missing $user/save (or legacy $root/save); run: just launch once, or copy a save there" >&2
    exit 2
fi
run="$root/.tmp/$name"
rm -rf "$run"
mkdir -p "$run/save"
cp -a "$save_src/." "$run/save/"
cd "$run"
export NSMBU_USER_DIR="${NSMBU_USER_DIR:-$user}"
export NSMBU_HIDDEN_WINDOWS=1
export NSMBU_NO_HOST_INPUT=1
export NSMBU_VK_PRESENT_MODE=immediate
export NSMBU_NO_AUDIO=1
export NSMBU_RES_SCALE="${NSMBU_RES_SCALE:-1}"
exec "$root/build/nsmbu" --game "$root/game" --save "$run/save"
