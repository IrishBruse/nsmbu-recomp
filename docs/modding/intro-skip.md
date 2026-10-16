# Intro skip

Investigation for `modding/examples/example.intro-skip`.

Addresses are USA v1.3.0.

## What ships today

The example reads the Save file option.
That option is 1, 2, or 3.
The default is 1.
It reads the Course pack option.
The default is `1-1`.
An empty Course pack also uses `1-1`.
A non-empty pack calls `scene.load_pack` after the save opens.
While the boot GamePad setup popup is current (`file_select()`, factory `0x0298AD3C`), it pulses confirm with `nsmbu.Input.press("A", 3)` about every 30 logic steps.
After four pulses, if the popup is still current, it retries `scene.open_title()` until the title or map phase is current.
It waits out the boot logo.
`BootScene` does not leave until its own timer finishes.
It calls `apply_save(file)` once that scene is gone.
`file` is the option minus 1.
That call writes the selected save and does not run guest `0x029872BC`.
It then calls `scene.load_pack` once and stops when `course_task()` is true.
`nsmbu.Level.active()` is already true while the title scene is coming up, so the skip does not use it as the stop.

`Hooks.onSceneChange` only sets pending flags from factory transitions.
Do not call `Scene` transition helpers from that callback.
`Hooks.onLogicStep` runs `apply_save`, `load_pack`, and `open_title`, and pulses confirm while the file-select popup is pending.

1. On factory `0x0298AD3C`, pulse `press("A", 3)` every 30 logic steps (up to four times).
2. If the popup is still current after four pulses, retry `open_title()` until `title()` or `map_phase()` is true.
3. After the boot logo (`0x0202930C` is gone), call `apply_save(file)` until it returns true.
4. Call `scene.load_pack` once, then stop when factory `0x0202B4C8` is current or `course_task()` is true.

Do not call `open_file_select()` for this skip.
Forcing menu factory `0x0202B61C` early shows the time-attack trial layout.

A boot log with this skip shows:

1. `press A file_select` (and optionally `open_title`) while factory `0x0298AD3C` is current
2. Title or map phase
3. `apply save` for the configured slot (`apply_save(file)`)
4. `load_pack` for the configured pack (`1-1` by default)
5. `course_task()` or factory `0x0202B4C8`

## Why the menu path failed

`open_file_select()` used to request menu factory `0x0202B61C`.
That layout is the time-attack trial page when created early.
`0x0297D58C` enters the challenge-select state and opens `ChallengeSelectScene` (`0x024C11F4`).

## Boot tasks

| Factory | Header name | Runtime constant | Probe |
| --- | --- | --- | --- |
| `0x0202930C` | `BootScene` | `kCreatorBoot` | (boot) |
| `0x0297B9F8` | none | `kCreatorFlow` | `flow_phase()` |
| `0x0298AD3C` | none | `kCreatorFileSelect` | `file_select()` |
| `0x0202B51C` | `TitleScene` | `kCreatorTitle` | `title()` |
| `0x0202B6C4` | none | post-title select | — |
| `0x020292B8` | none | `kCreatorMap` | `map_phase()` |
| `0x0202B570` | none | pre-course | — |
| `0x0202B61C` | `MenuScene` | `kCreatorMenu` | do not force |
| `0x024C11F4` | `ChallengeSelectScene` | `kCreatorSaveSelect` | `save_select()` |
| `0x0202B4C8` | `CourseTask` | `kCreatorCourseTask` | `course_task()` |
| `0x0202B86C` | `CourseSelectTask` | `kCreatorCourseSelect` | `course_select()` |

Observed boot order from `Scene::setNextScene` logs:

1. `BootScene` `0x0202930C`
2. Flow `0x0297B9F8`
3. File select `0x0298AD3C` (GamePad setup popup)
4. Title `0x0202B51C`
5. After `apply_save`: `CourseSelectTask` `0x0202B86C` (story path may skip straight to `CourseTask`)

## Recommendation (current skip)

While `file_select()` is true, pulse `press("A", 3)` then fall back to `open_title()` if needed.
Call `apply_save(file)` from the title or map phase after the boot logo.
`file` is 0, 1, or 2.
Then call `scene.load_pack`.
The default pack is `1-1`.
Stop on `course_task()`.
Do not force `0x0202B61C`.

---

## Open: skip the GamePad popup

### What the popup is

The boot GamePad / controller setup screen is **not** `MenuScene`.
It is the unnamed file-select task at factory `0x0298AD3C` (`kCreatorFileSelect`).
`scene.file_select()` is true while that task is current.
It sits after the application flow task `0x0297B9F8` (`scene.flow_phase()`) and before `TitleScene`.

`MenuScene::enterFileState` (`0x0297D268`) is the later save-file menu.
Do not use `open_file_select()` or `confirm_file()` for this popup.

`MainGame.mIsSinglePlayerDRC` (`MainGame::sInstance` `0x101E7D7C` + `0x14`) is the GamePad vs Pro flag for single-player input after the map loads.
`apply_save` already writes that field from the host Pro / GamePad setting.
That write does not dismiss the boot popup.

### What shipped vs what is still open

The example now uses approach **A** while `file_select()` is true: `nsmbu.Input.press("A", 3)` about every 30 logic steps, up to four pulses.
If the popup is still current, it falls back to `scene.open_title()` (the existing SDK stand-in for approach **B**), with the same retry pattern as `apply_save`.
It does not add a native `skip_setup()` helper (approach **C**).

Which button the layout actually consumes is still unknown.
**A** is the first guess; Plus or a layout-specific decide path may be required.
Log lines from real boots should confirm whether **A** alone clears the window or `open_title` is doing the work.

Headless or `NSMBU_NO_HOST_INPUT` boots used to stall on `wait boot` at phase `0298AD3C` before this path existed.
Some interactive boots still clear the popup without mod help in a few logic steps.
That behaviour is not reliable enough for automation on its own.

### UI path (partial RE)

Generated code around the file-select task uses:

| Address | Role |
| --- | --- |
| `0x024BC7D4` | `CommonWindowMgr::processPadTrig_OneButton` |
| `0x025F8788` | `PaCursor::triggerDecide` |
| flow object `+0x12C` | gate byte set to `1` on decide (seen at `0x0298A4C4`) |

Decomp layout types include `CommonWindow::cType_WinText_SetUpBtn` in `decomp/include/layout/CommonWindow.h`.
The flow / file-select task classes themselves are not named in `decomp/` yet.

### Options to implement

| Approach | How | Pros | Cons |
| --- | --- | --- | --- |
| **A. Input inject** | While `scene.file_select()`, call `nsmbu.Input.press("A", n)` (or the confirm button the layout uses) until `title()` | No new SDK; matches real UI | Need the right button and a few steps; fragile if layout changes |
| **B. Force title** | From `file_select()`, call `Scene::setNextScene` with `kCreatorTitle` when the fader is idle | Fast; no pad | No Luau wrapper for an arbitrary factory today; must respect `scene busy` / fader |
| **C. Native helper** | Add e.g. `scene.skip_setup()` that sets the known gate / advances state, or wraps B | Clean API for the example | Needs a small runtime change and a short RE pass on the decide path |
| **D. Leave as-is** | Keep waiting for title / map | Zero work when the popup auto-clears | Breaks scripted boots that need no host input |

### Recommended next steps

1. Confirm from logs whether **A** or `open_title` clears the popup on your boot path.
2. If **A** does not advance the window, try other buttons (Plus, etc.) or add approach **C** only if `open_title` is insufficient.
3. Keep writing `mIsSinglePlayerDRC` through `apply_save` so the map matches host GamePad vs Pro.

Do not use `open_file_select()`, `0x0202B61C`, or `0x0297D58C` for this screen.

---

## Course load (`load_pack`)

Goal example: enter the stage that loads `game/content/Common/course_res_pack/1-23.szs` (pack name `1-23`) without stopping on the world map.

### Pack name mapping (proven)

`Scene::setCourse_Story` (`0x029941D0`) writes `CourseInfo::setNextWorldNo` (`+0x1C`) and `setNextCourseNo` (`+0x1D`), then requests `CourseTask` `0x0202B4C8`.

Course setup `f_024BD86C` calls `f_024D29A4`, which copies those next fields to `+0x10` and `+0x11` on `CourseInfo` (`sInstance` `0x101D18EC`).

Later in that function, `f_0269A2B4` sprintfs format `0x100E6040` (`"%d-%d"`) as `(byte+0x10)+1` and `(byte+0x11)+1`.
That string is the `%s` in `course_res_pack/%s.szs` (`f_029D3150`).
Pack `1-23` is `load_story(0, 22)` or `load_pack("1-23")`.
`course_no` is `pack_id - 1`, not a separate map slot index.

Guest `0x024D3AD4` is a save-nibble helper.
It is not the filename builder.
Older notes that pointed at ROM tables near `0x100C97xx` for the pack filename were wrong.

The Luau API exposes:

```luau
scene.load_story(world_no, course_no)
scene.load_pack("1-23")
```

Both call `setCourse_Story` when the fader is idle.
The second argument is the live flow task.
A null source crashes inside `Scene::setNextScene` (`ld32` at `0x029947D0`).
They return `"scene busy"` while a scene change is queued or the fader is busy.
The example requests the pack once, then waits for `course_task()`.

`load_pack` shipped in the runtime.
The example uses it with the Course pack option.

### Normal enter path

1. Intro skip → `apply_save` → optional `load_pack` → `CourseTask`.
2. Player picks a node on the map (vanilla path).
3. `setCourse_Story` → `CourseTask` `0x0202B4C8`.
4. Course setup calls `loadCourseResPack` with the resolved name (e.g. `1-23`).

Save-select story confirm (`MenuScene::confirmFile` with story gate) also ends on `CourseTask` `0x0202B4C8` after copying save into `CourseInfo`.
Challenge / time-attack uses other course indices and is the wrong path for story packs like `1-23`.

Do not call `ResMgr::loadCourseResPack` from a mod without the `setCourse_Story` → `CourseTask` path.
Do not force `ChallengeSelectScene` for story packs.

---

## Address quick reference

| Item | Address |
| --- | --- |
| `Scene::setNextScene` | `0x02994164` |
| `Scene::setCourse_Story` | `0x029941D0` |
| Save continue → map | `0x029872BC` |
| Apply save file | `0x029D7648` |
| Boot file-select factory | `0x0298AD3C` |
| Flow factory | `0x0297B9F8` |
| `CourseSelectTask` factory | `0x0202B86C` |
| `CourseTask` factory | `0x0202B4C8` |
| `CourseInfo::sInstance` | `0x101D18EC` |
| `MainGame::sInstance` | `0x101E7D7C` |
| `mIsSinglePlayerDRC` | instance + `0x14` |
| CourseInfo copy for pack name | `0x024D29A4` |
| Pack name sprintf | `0x0269A2B4` |
| `ResMgr::loadCourseResPack` (guest) | `0x029D30EC` |
