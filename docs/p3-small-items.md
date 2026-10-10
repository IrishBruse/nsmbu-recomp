# P3 runtime changes

This tree is New Super Mario Bros. U.
The Wind Waker run and swim speed checks, surface timings, and ZeldaWWHDRecomp CI links are not used here.
NSMBU logic stays at 60 steps a second.
The notes below are the P3 behaviors that still match this port.

## Average performance overlay

`runtime/src/overlay/perf_average.h` stores the average frame rate and the average logic steps.
`runtime/src/overlay/overlay.cpp` updates those averages on every frame.
The update still runs when the overlay is hidden.
The average resets when the renderer, the internal resolution, the frame mode, or the target rate changes.
It also resets when the frame count or the logic-step count goes backwards.
The counters are presented frames (`gx2::flips_presented`) and logic steps (`interp::logic_steps`).
`runtime/src/overlay/android_telemetry.h` reads GPU busy time and CPU, GPU, and SoC temperatures.
`overlay.cpp` polls that data every two seconds while the performance overlay is visible.
A missing reading is left out.
Temperature files use Linux millidegrees Celsius.
The value on screen is that number divided by 1000.
`runtime/tools/perf_average_test.cpp` checks the average and a synthetic kgsl file.

## Crash context and Android sharing

`runtime/src/crash_context.cpp` keeps a bounded snapshot.
`runtime/src/crashrec.cpp` refreshes that snapshot once a second on the game thread.
A startup snapshot is stored before the game starts.
The snapshot names the renderer, the resolution, the frame mode, the target rate, and the controller.
It also lists built-in switches, enabled packages, and `NSMBU_` environment variables.
A variable name that contains KEY, TOKEN, SECRET, or PASSWORD is stored as `<redacted>`.
The snapshot uses atomic bytes and a revision count.
A crash handler reads it with no lock and no allocation.
An interrupted update is reported as interrupted.
`runtime/src/crash_redact.h` replaces home paths and user names in crash text.
`runtime/src/main.cpp` uses that redaction for the crash log and the saved log ring.
`runtime/src/hle/coreinit_misc.cpp` uses it for guest halt logs.
`android/app/src/main/java/org/nsmbrecomp/nsmbu/NsmbuActivity.java` offers the newest crash report once on the next start.
The share copy is placed in the app cache directory `crash-share`.
`android/app/src/main/res/xml/crash_share_paths.xml` limits the FileProvider to that directory.
The share intent grants read access to that copy.
The player must tap Share.
`runtime/tools/crash_redact_test.cpp` checks the redaction.
