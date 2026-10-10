---
name: upstream-merge
description: Take a newer Wind Waker HD recomp commit into this NSMBU tree with one merge.
disable-model-invocation: true
---

# Upstream merge

One `git merge upstream/main`.

A rebase replays every NSMBU commit and conflicts the same files again.

A squash is one pass this time and a full pass on every later update, because it stores no upstream parent.

## Steps

1. Stop when `git status` shows staged or unstaged changes.
2. Run `git fetch upstream main`.
   When the fetch says the clone is shallow, run `git fetch --unshallow upstream`, then fetch `main` again.
3. Run `git merge upstream/main`.
   This branch is ours (NSMBU).
   `upstream/main` is theirs.
4. Resolve every conflict with the rules below.
   Run `git add` on a file only after `git diff` on that file shows no conflict markers.
5. `git diff --check` is clean.
   `git grep -n '<<<<<<<' -- ':!runtime/third_party' ':!references'` prints nothing.
6. Replace `docs/upstream-wwhd-readme.md` with `git show upstream/main:README.md`.
   Leave Wind Waker names in that file.
7. Set the recorded base in `docs/nsmbu.md` and the last-merge note in `README.md` to `git rev-parse upstream/main`, plus that commit's subject and author date.
   In `README.md`, keep the sentence form `The last merge from [ZeldaWWHDRecomp](https://github.com/ZeldaWWHDRecomp/ZeldaWWHDRecomp) is commit \`<hash>\`.`
8. When `build/Makefile` exists, run `make -C build -j"$(nproc)" nsmbu`.
9. Finish the merge with those doc files included.
   Do not push.

## Conflict rules

Keep upstream engine behavior.

Apply the name map to new upstream text.

Keep a new upstream engine feature that shares a hunk with a Wind Waker deletion.

Wind Waker product features stay deleted: built-in cheats, climb, camera, turbo, GameCube save import, `tools/savegame`, and Wind Waker hook lists.

Use NSMBU guest ABI names (`NSMBU_GUEST_*`, `nsmbu_guest.h`, `NSMBUGuest*`) in this fork.

Rename a new environment variable to `NSMBU_` only when this tree's runtime already reads the `NSMBU_` form.

## Name map

Apply in this order.

1. `The Legend of Zelda: The Wind Waker HD` to `New Super Mario Bros. U`
2. `The Wind Waker HD` to `New Super Mario Bros. U`
3. `Wind Waker HD` to `NSMBU`
4. `WindWakerHD` to `NSMBU`
5. `wind-waker-hd` to `nsmbu-launcher`
6. `ZeldaWWHDRecomp` to `NSMBURecomp`
7. `zeldawwhdrecomp` to `nsmbrecomp`
8. `cking.rpx` to `red-pro2.rpx`
9. `WWHD` to `NSMBU`
10. `wwhd` to `nsmbu`

The bundle id is `io.github.nsmbrecomp.nsmbu`.

Step 7 comes before step 10.

The Android package is `org.nsmbrecomp.nsmbu`.

Leave the GitHub path `GreenNaugahyde/ZeldaWWHDRecompAndroid`.

Names already in this tree: executable `nsmbu`, Windows `NSMBU.exe`, launcher `nsmbu-launcher`, RPX `red-pro2.rpx`, XDG dir `nsmbu`, log `captures/nsmbu.log`, setup log `nsmbu-setup-log.log`.
