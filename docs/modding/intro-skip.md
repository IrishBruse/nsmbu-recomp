# Intro skip

Investigation for `modding/examples/example.intro-skip`.

The example stops on the first save slot.
It does not open the time-attack page.
The example sends `open_file_select()` again when the call returns `"scene busy"` or `"file not ready"`.
It stops when `file_index()` is 0.
It treats `save_select()` as the time-attack path and does not stop there.

This note records the boot tasks and the other ways that were considered.

Addresses are USA v1.3.0.
Names come from `references/headers`.
Bodies come from the generated C in `build/gen`.
`decomp/include/scene/` holds `BootScene`, `ChallengeSelectScene`, and `MenuTask0202B61C`.

## What the example does

`hooks.on_logic_step` runs this sequence.

1. While `scene.title()` or `scene.map_phase()` is true, call `scene.open_file_select()`.
   Advance when the call returns true.
   A boot log shows `setNextScene` request TitleScene `0x0202B51C`, then the current phase stays on AreaTask `0x020292B8`.
   `title()` stays false on that boot.
   `map_phase()` is that AreaTask.
2. Read `scene.file_index()`.
   Stop when the number is 0.
   Do not call `confirm_file()` or `open_course_select()`.
   Do not treat `save_select()` as success.
   `save_select()` means `ChallengeSelectScene` (`0x024C11F4`), not the save-slot page.
   The menu task can own the save-slot state while `phase()` is still another factory.

Each transition returns `nil` and `"scene busy"` while `mNextScene` is set or the fader is busy.
`open_file_select()` also returns `nil` and `"file not ready"` when the file is not ready.
The runtime makes that check before it calls the guest function.
A retry does not play the title sound and does not clear `SubjectMgr` again.
The script sends the same call on the next logic step.

## When a scene change is accepted

`Scene::setNextScene` calls `0x0299477C` first.
That function returns false in two cases.

- `Scene + 0x5C` is not zero.
  That word is `mNextScene` type.
  A scene change is already queued.
- The fader at `Scene + 0x68` reports busy.

`Scene` is `0x84` bytes (`references/headers/scene/Scene.h`).
`mNowScene` factory is at `+0x58`.
`mNextScene` type is at `+0x5C`.
`mFader` is at `+0x68`.
`nsmbu.scene.phase()` reads the factory at `+0x58`.

A later call can succeed on a later step.
The script must keep the same call until it returns true.
`open_file_select()` does not request a factory.
The skip does not wait for phase `0x024C11F4`.

## Boot tasks

Each value below is a `sead::TTaskFactory`.
The factory allocates the task and calls the constructor.
`setNextScene` stores the factory in `mNowScene`.

| Factory | Size | Constructor | Header name | Runtime constant |
| --- | --- | --- | --- | --- |
| `0x0202930C` | `0x103C` | `0x0297AC48` | `BootScene` | `kCreatorBoot` |
| `0x0202B51C` | title task | `TitleScene::TitleScene` | `sead::TTaskFactory<TitleScene>` | `kCreatorTitle` |
| `0x0202B61C` | `0x3E94` | `0x0297C5D8` | `MenuTask0202B61C` | `kCreatorMenu` |
| `0x0297B9F8` | `0x2C4` | `0x02989CF8` | none | `kCreatorFlow` |
| `0x024C11F4` | `0x11D0` | `0x0297F3CC` | `ChallengeSelectScene` | `kCreatorSaveSelect` |
| `0x0298AD3C` | `0x138` | `0x02990610` | none | `kCreatorFileSelect` |
| `0x0202B4C8` | `0x5A4` | `CourseTask::CourseTask` | none in the names file | none |
| `0x020292B8` | `0xBE4` | `AreaTask::AreaTask` | none in the names file | `kCreatorMap` |
| `0x0202B86C` | `0xDD0` | `CourseSelectTask::CourseSelectTask` | `sead::TTaskFactory<CourseSelectTask>` | `kCreatorCourseSelect` |

`TitleScene`, `BootScene`, `ChallengeSelectScene`, `MenuTask0202B61C`, `CourseTask`, `AreaTask`, and `CourseSelectTask` are in `decomp/include/scene/` (and related paths).
`MenuTask0202B61C` is the address-based name for factory `0x0202B61C`.
The headers tree has no RTTI name for that task.
`0x024C11F4` is `ChallengeSelectScene`, the time-attack scene.
Factories `0x0297B9F8` and `0x0298AD3C` still have no header name.
Keep the address until a header names them.

`nsmbu.scene.file_select()` is true when `mNowScene` is `0x0298AD3C`.
`0x024C11F4` is `ChallengeSelectScene`.
That scene is the time-attack page.
`open_file_select()` does not request that factory.
`save_select()` is true when the current scene factory is `0x024C11F4`.
The skip does not wait on that phase.
`open_file_select()` requests factory `0x0202B61C` when that menu task is missing.
It then calls `0x024CE594` on `SubjectMgr` (`0x101D1874`) with file 0.
It then calls `MenuTask0202B61C::enterSaveSlotPage` (`0x0297D58C`) on the menu task.
That call only runs `StateMethod::changeStateMethod` on `+0x1DC` with state id `0x1022C184`.
It enters the save-slot page.
Do not call `MenuTask0202B61C::openChallengeSelect` (`0x0297D5A8`).
That function plays a title sound effect, clears `SubjectMgr`, and requests `ChallengeSelectScene` (`0x024C11F4`).
That path is time attack, not the save-slot state.
The flow task (`0x0297B9F8`, size `0x2C4`) does not own that state.

`course()` reads `CourseTask::sInstance` (`0x101D15F0`).
`map_phase()` is true when `mNowScene` is the `AreaTask` factory.
`course_select()` reads `CourseSelectTask::sInstance` (`0x101CC96C`).
The constructor at `0x022656E0` sets that singleton.
The world map is up when `course_select()` becomes true.

## The three calls

The source task for `setNextScene` is a live task.
`Scene` is not that task.
A task stores its virtual table at `+0x70`.
The flow task uses virtual table `0x10177E4C`.
`AreaTask::sInstance` is `0x101D1598`.
The runtime looks for that task on the scene task lists and in the task tree.
It also keeps the source of the title request while that object stays on a list.
It does not pass `AreaTask`.
The port passes that object as `this` and as the source task.
The save-slot menu is factory `0x0202B61C` (size `0x3E94`, virtual table `0x10175F88`).
Offsets `0xDB1`, `0x1CAD`, `0x1DC`, and `0x3E90` are on that menu task.

| Offset | Use |
| --- | --- |
| `+0xDB1` | Select gate. `confirm_file` returns at once when this byte is 0. |
| `+0xDB2` | Story gate. A non-zero value takes the world-map path. |
| `+0x1CAD` | Set to 1 after a successful confirm. |
| `+0x1DC` | `StateMethod` (`0x10` bytes). Confirm calls `StateMethod::changeStateMethod` with state id `0x1022C1D0`. |

### `open_file_select`

`open_file_select()` selects save file 0 and enters the save-slot page.
It does not request factory `0x024C11F4`.
`0x024C11F4` is `ChallengeSelectScene`, the time-attack scene.

1. Request factory `0x0202B61C` when the menu task is missing.
2. Call `0x024CE594` on `SubjectMgr` (`0x101D1874`) with file 0.
3. Call `0x0297D58C` on the menu task.
   That call changes the menu state at `+0x1DC` to `0x1022C184`.

`SubjectMgr::mSelectedFile` is the byte at `+0x1E7E`.
`SubjectMgr::mMode` is the byte at `+0x1E7F`.
`SubjectMgr::clearSubject` (`0x024CD904`) sets `mMode` to 0.
`SubjectMgr::selectSaveFile` (`0x024CE594`) sets `mMode` to `cMode_Active` (1) when the save record for the file exists, and it stores the file in `mSelectedFile`.
The constructor stores 0 in `mSelectedFile`, so a range check on that byte alone is not a ready signal.
`nsmbu.scene.file_index()` returns the file only while `mMode` is active.

### `confirm_file` — `0x0297EC3C`

The port sets both gate bytes to 1, then calls this function.

When the story gate is set, the function does this.

1. Read `SubjectMgr + 0x1E7E`.
2. Call `0x024CE594` on `SubjectMgr` with that byte.
3. Call `0x0297EAAC`.
4. `CourseInfo::setNextSceneEnterType(1)` (`0x024D299C`).
5. `Scene::setNextScene` to the `CourseTask` factory `0x0202B4C8`.
6. `StateMethod::changeStateMethod`.

`0x0297EAAC` copies the chosen save into `CourseInfo`.
During boot, `CourseTask::sInstance` is null, so it uses `FieldGame::sInstance + 0x10` (`0x101D1604`).
It calls `0x024C2E3C(SubjectMgr + 0x10, file index)`.
It then stores two bytes from that record:

- `+0x43` through `CourseInfo::setNextFileNo`
- `+0x47` through `CourseInfo::setNextNextGotoNo`

A confirm that runs before the save-select task writes `+0x1E7E` copies an empty index.
The map then opens on the wrong file.

When the story gate is 0, the same function requests factory `0x024C11F4` again.
That factory is `ChallengeSelectScene`, the time-attack scene, so the story gate must stay 1.

### `open_course_select`

The port calls `Scene::setNextScene` with the `CourseSelectTask` factory and fader `0x000A000A`.
`Scene::FaderInfo` stores the out type in the high 16 bits and the in type in the low 16 bits.
Both types are 10.

`confirm_file` requests `CourseTask`.
`CourseSelectTask` is a second task.
The world map is interactive only after that second task exists.
`setNextScene` refuses the second request while `CourseTask` still owns the fader or `mNextScene`.

## Ways to implement the skip

### 1. Retry each call until the scene shows it

This table is the old three-call path.
The current example does not wait on `save_select()`.

| Step | Send the call when | Call | Advance when |
| --- | --- | --- | --- |
| 1 | `title()` or `map_phase()` is true | `open_file_select()` | The call returns true |
| 2 | `save_select()` is true and `file_index()` returns a file | `confirm_file()` | The call returns true |
| 3 | `course()` and `course_task()` are true | `open_course_select()` | The call returns true |
| 4 | `course_select()` is true | none | Stop |

`save_select()` is true when the current scene factory is `0x024C11F4`.
`0x024C11F4` is `ChallengeSelectScene`, the time-attack scene.
The example does not treat `save_select()` as success.
`course_task()` is factory `0x0202B4C8`.
`file_index()` reads `SubjectMgr + 0x1E7E` while `SubjectMgr::mMode` is active.

The example stops when `file_index()` is 0.

### 2. Press A through `nsmbu.input.press`

The title and the save select already handle A.
A script can press A while `title()` is true, then press A again after the save select has a file index, then press A if the map still waits.

The press still needs the same ready signals as way 1.
The press also depends on which button the layout reads, and on focus.
The scene calls skip that layout.
The current example stops on save slot 0.

### 3. One `setNextScene` from the title to `CourseSelectTask`

`open_course_select()` can run from the title when `mNextScene` is empty and the fader is idle.
That call creates `CourseSelectTask`.
It skips `0x0297EAAC`.
`CourseInfo` then has no file, no goto, and enter type 1 from the confirm path.
The map has no save.

A single jump is valid only when the script runs the `0x0297EAAC` setup first, then requests `CourseTask`, then requests `CourseSelectTask` after that task accepts a new scene.
That is way 1 with the save-select task removed.
The save-select task is what writes `SubjectMgr + 0x1E7E`.
Remove it only after another function is shown to write that byte from `SaveData::Header::last_file`.

## Recommendation

The example stops on save slot 0.
`file_index()` is the probe for that stop.
`save_select()` is the time-attack probe.
The example does not treat it as success.
`open_file_select()` returns `"scene busy"` or `"file not ready"` when it cannot run.
The script sends the same call again.

Leave way 3 until a boot log shows a writer for `SubjectMgr + 0x1E7E` outside `0x024CE594`.
The `CourseSelectTask` constructor hook `hook_022656E0` records the caller of a natural start.
One boot with the example off fills in that caller.

## Gaps

- Factories `0x0297B9F8` and `0x0298AD3C` have no header name.
- `MenuTask0202B61C` is a decomp placeholder name for factory `0x0202B61C`.
- State id `0x1022C1D0` has no header name.
- `references/decomp` does not contain these functions.
- `SaveMgr::sInstance` has no address in the headers.
