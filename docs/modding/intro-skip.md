# Intro skip

Investigation for `modding/examples/example.intro-skip`.

The example reads the Save file option.
That option is 1, 2, or 3.
The default is 1.
The example calls `open_first_save(file)` once the title or the map phase is current.
`file` is the option minus 1.
That call writes the selected save file and runs guest `0x029872BC`.
`0x029872BC` requests `CourseSelectTask` `0x0202B86C` when the file has a record.
An empty file follows the game branch to factory `0x0202B570`.
The example stops when `course_select()` is true.
It does not press buttons.

Addresses are USA v1.3.0.

## What the example does

`hooks.on_logic_step` runs this sequence.

1. Wait until `title()` or `map_phase()` is true.
2. Read the Save file option and call `open_first_save(file)` until it returns true.
3. Stop when `course_select()` is true.

Do not call `open_file_select()` for this skip.
Forcing menu factory `0x0202B61C` early shows the time-attack trial layout.

A boot log with this skip shows:

1. Title or map phase
2. `open_first_save(file)`
3. `CourseSelectTask` `0x0202B86C`
4. `course_select()` becomes true

## Why the menu path failed

`open_file_select()` used to request menu factory `0x0202B61C`.
That layout is the time-attack trial page when created early.
`0x0297D58C` enters the challenge-select state and opens `ChallengeSelectScene` (`0x024C11F4`).

## Boot tasks

| Factory | Header name | Runtime constant |
| --- | --- | --- |
| `0x0202B51C` | `TitleScene` | `kCreatorTitle` |
| `0x0202B6C4` | none | post-title select |
| `0x0202B570` | none | pre-course |
| `0x0202B61C` | `MenuScene` | `kCreatorMenu` (do not force) |
| `0x024C11F4` | `ChallengeSelectScene` | `kCreatorSaveSelect` |
| `0x0202B4C8` | `CourseTask` | course body |
| `0x0202B86C` | `CourseSelectTask` | `kCreatorCourseSelect` |

## Recommendation

Call `open_first_save(file)` from the title or map phase.
`file` is 0, 1, or 2.
Stop on `course_select()`.
Do not force `0x0202B61C`.
