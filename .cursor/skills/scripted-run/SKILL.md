---
name: scripted-run
description: Boot NSMBU from a copy of save/ and dump frames with tools/scripted-run.sh.
---

# Scripted run

Dumps are `frame_N.png` in the process working directory.
`tools/scripted-run.sh` sets that directory to `.tmp/<name>` and copies `save/` to `.tmp/<name>/save`.
The repo `save/` stays unread by the game.

## Steps

1. Set `NSMBU_EXIT_AT_FRAME`.
   Set `NSMBU_DUMP_FRAMES` to the frames you will look at.
   Set `NSMBU_PRESS` and `NSMBU_STICK` when the boot must press buttons.
   `NSMBU_RES_SCALE` defaults to 1.
2. Run `tools/scripted-run.sh <name>` from the repo root.
   The run is done when the process exits and each dumped `frame_<n>.png` is in `.tmp/<name>`.
3. Read one dumped frame.
   Continue only when that frame is the scene you mean.
4. For a shake check, dump the same consecutive frames at scale 1 and at scale 2.
   Run `python3 tools/flicker.py FIRST LAST .tmp/<name>`.
   The check is done when both scales have a result and you have looked at one frame from each.

## Input

`NSMBU_PRESS` holds VPAD bits for a TV-frame range.
`8000` is A.
Use a short pulse, about 20 frames.
A hold that is already down when a dialog opens does not press the button again.

`NSMBU_STICK` is `from-to:x:y` on the left stick.
Positive x is right.
Positive y is up.

## World 1-1

`save/` is already past the opening cutscene.
This recipe starts from that save.

```
NSMBU_PRESS='1800-1820:8000,2600-2620:8000,3400-3420:8000,4200-4220:8000,5000-5020:8000,6300-6340:8000' \
NSMBU_STICK='5400-6000:1:0' \
NSMBU_DUMP_FRAMES=7000 \
NSMBU_EXIT_AT_FRAME=7100 \
tools/scripted-run.sh w11
```

Frame 7000 is the course.
The picture is the big tree, the bricks, and Mario on the ground.
A Goomba reaches idle Mario and the game returns to the map before frame 7800.
Dump the still frames in that gap.

## Mute

The script sets `NSMBU_NO_AUDIO=1` so no output device is opened.
The mix still runs.
Interactive mute is in the settings overlay (F1) > Audio.
