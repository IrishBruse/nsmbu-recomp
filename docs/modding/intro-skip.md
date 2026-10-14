# Intro skip

Investigation for `modding/examples/example.intro-skip`.

The example loads the world map with three scene calls.
The calls match the game.
The example sends each call again while `Scene::setNextScene` would refuse it.
It advances when the next boot probe is true.

This note records the boot tasks and the other ways that were considered.

Addresses are USA v1.3.0.
Names come from `references/headers`.
Bodies come from the generated C in `build/gen`.
`references/decomp` has no scene, title, or save-select sources.

## What the example does

`hooks.on_logic_step` runs this sequence.

1. While `scene.title()` or `scene.map_phase()` is true, call `scene.open_file_select()`.
   Advance when the call returns true.
   A boot log shows `setNextScene` request TitleScene `0x0202B51C`, then the current phase stays on AreaTask `0x020292B8`.
   `title()` stays false on that boot.
   `map_phase()` is that AreaTask.
2. While `scene.save_select()` is true and `scene.file_index()` returns a file, call `scene.confirm_file()`.
   Advance when the call returns true.
3. While `scene.course()` and `scene.course_task()` are true, call `scene.open_course_select()`.
4. Stop when `scene.course_select()` is true.

Each transition returns `nil` and `"scene busy"` while `mNextScene` is set or the fader is busy.
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
The script must keep the same call until it returns true, and until `phase()` shows the requested factory.

## Boot tasks

Each value below is a `sead::TTaskFactory`.
The factory allocates the task and calls the constructor.
`setNextScene` stores the factory in `mNowScene`.

| Factory | Size | Constructor | Header name | Runtime constant |
| --- | --- | --- | --- | --- |
| `0x0202930C` | `0x103C` | `0x0297AC48` | none | `kCreatorBoot` |
| `0x0202B51C` | title task | `TitleScene::TitleScene` | `sead::TTaskFactory<TitleScene>` | `kCreatorTitle` |
| `0x0297B9F8` | `0x2C4` | `0x02989CF8` | none | `kCreatorFlow` |
| `0x024C11F4` | `0x11D0` | `0x0297F3CC` | none | none |
| `0x0298AD3C` | `0x138` | `0x02990610` | none | `kCreatorFileSelect` |
| `0x0202B4C8` | `0x5A4` | `CourseTask::CourseTask` | none in the names file | none |
| `0x020292B8` | `0xBE4` | `AreaTask::AreaTask` | none in the names file | `kCreatorMap` |
| `0x0202B86C` | `0xDD0` | `CourseSelectTask::CourseSelectTask` | `sead::TTaskFactory<CourseSelectTask>` | `kCreatorCourseSelect` |

`TitleScene`, `CourseTask`, `AreaTask`, and `CourseSelectTask` are in the headers.
The other four factories have no header name.
Keep the address until a header names them.

`nsmbu.scene.file_select()` is true when `mNowScene` is `0x0298AD3C`.
`open_file_select()` requests `0x024C11F4`.
Those are different tasks.
`file_select()` stays false through the save select that this skip opens.
Wait on `phase()` equal to `0x024C11F4`.

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
The object is the `0x2C4` flow task.
These fields fit in that size.

| Offset | Use |
| --- | --- |
| `+0xDB1` | Select gate. `confirm_file` returns at once when this byte is 0. |
| `+0xDB2` | Story gate. A non-zero value takes the world-map path. |
| `+0x1CAD` | Set to 1 after a successful confirm. |
| `+0x1DC` | `StateMethod` (`0x10` bytes). Confirm calls `StateMethod::changeStateMethod` with state id `0x1022C1D0`. |

### `open_file_select` — `0x0297D5A8`

This is the call the title makes for the save select.

1. `SndAudioMgr::startSystemSe` (`0x029B54C0`).
2. `0x024CD904` on `SubjectMgr::sInstance` (`0x101D1874`).
   That function clears `SubjectMgr` fields from `+0x1E28` through the file byte at `+0x1E7E`.
3. `Scene::setNextScene` to factory `0x024C11F4`, fader 0.

`SubjectMgr::mMode` is the byte at `+0x1E7F`.
`0x024CD904` sets that byte to 0.
`0x024CE594` sets it to `cMode_Active` (1) when the save record for the file exists, and it stores the file at `+0x1E7E`.
The constructor stores 0 at `+0x1E7E`, so a range check on that byte is not a ready signal.
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
That is the save-select task, so the story gate must stay 1.

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

Keep the three calls.
Remove the step counts.

| Step | Send the call when | Call | Advance when |
| --- | --- | --- | --- |
| 1 | `title()` or `map_phase()` is true | `open_file_select()` | The call returns true |
| 2 | `save_select()` is true and `file_index()` returns a file | `confirm_file()` | The call returns true |
| 3 | `course()` and `course_task()` are true | `open_course_select()` | The call returns true |
| 4 | `course_select()` is true | none | Stop |

`save_select()` is factory `0x024C11F4`.
`course_task()` is factory `0x0202B4C8`.
`file_index()` reads `SubjectMgr + 0x1E7E` while `SubjectMgr::mMode` is active.

The example uses this path.

### 2. Press A through `nsmbu.input.press`

The title and the save select already handle A.
A script can press A while `title()` is true, then press A again after the save select has a file index, then press A if the map still waits.

The press still needs the same ready signals as way 1.
The press also depends on which button the layout reads, and on focus.
The scene calls skip that layout.
Way 1 is the stable form of the current example.

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

The example uses way 1.
`save_select()`, `course_task()`, and `file_index()` are the probes.
The three transition calls return `"scene busy"` until `Scene::setNextScene` can accept them.

Leave way 3 until a boot log shows a writer for `SubjectMgr + 0x1E7E` outside `0x024CE594`.
The `CourseSelectTask` constructor hook `hook_022656E0` records the caller of a natural start.
One boot with the example off fills in that caller.

## Gaps

- Factories `0x0202930C`, `0x0297B9F8`, `0x024C11F4`, and `0x0298AD3C` have no header name.
- `SubjectMgr + 0x1E7E` has no header name. The runtime constant is `kSubjectFileOff`. `+0x1E7F` is `SubjectMgr::mMode`.
- State id `0x1022C1D0` has no header name.
- `references/decomp` does not contain these functions.
- `SaveMgr::sInstance` has no address in the headers.
