# Intro skip

Investigation for `modding/examples/example.intro-skip`.

The example presses A through the title path until the world map is up.
It does not open the menu task or the time-attack page.

Addresses are USA v1.3.0.

## What the example does

`hooks.on_logic_step` runs this sequence.

1. Wait until the title path is current (`title()`, `map_phase()`, or phase `0x0202B6C4` / `0x0202B570`).
2. Press A on a timer.
3. Stop when `course_select()` is true.

Do not call `open_file_select()` for this skip.
Forcing menu factory `0x0202B61C` early shows the time-attack trial layout.

A boot log with this skip shows:

1. Title factory `0x0202B51C`
2. After A, factory `0x0202B6C4`
3. After A, `CourseSelectTask` `0x0202B86C`
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
| `0x0202B61C` | `MenuTask0202B61C` | `kCreatorMenu` (do not force) |
| `0x024C11F4` | `ChallengeSelectScene` | `kCreatorSaveSelect` |
| `0x0202B4C8` | `CourseTask` | course body |
| `0x0202B86C` | `CourseSelectTask` | `kCreatorCourseSelect` |

## Recommendation

Press A through the title path.
Stop on `course_select()`.
Do not force `0x0202B61C`.
