
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
name=${1:-survey}
if [ -z "${NSMBU_EXIT_AT_FRAME:-}" ]; then
    echo "Set NSMBU_EXIT_AT_FRAME" >&2
    exit 2
fi
run="$root/.tmp/$name"
rm -rf "$run"
mkdir -p "$run/save"
cp -a "$root/save/." "$run/save/"
cd "$run"
export NSMBU_HIDDEN_WINDOWS=1
export NSMBU_NO_HOST_INPUT=1
export NSMBU_VK_PRESENT_MODE=immediate
export NSMBU_NO_AUDIO=1
export NSMBU_RES_SCALE="${NSMBU_RES_SCALE:-1}"
exec "$root/build/nsmbu" --game "$root/game" --save "$run/save"
