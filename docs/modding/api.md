# Luau API (version 1)

Status: **Phase 2 partial**.
Lifecycle fields on `nsmbu.hooks` (`on_loaded`, `on_logic_step`, `on_config_changed`, and `on_unload`), package metadata, log and status, time, config, guest memory, high-level `nsmbu.level` / `nsmbu.player` helpers, level actor reads / spawns, and `nsmbu.scene` flow probes and transitions are implemented with embedded Luau.
Function hooks (`hook` / `replace` / `call` on `nsmbu.hooks`), input pad reads, data files, guest heap alloc, and `nsmbu.fn` stay proposal until later phases.
Package plan: [luau-mods.md](luau-mods.md).
Cemu packs do not use this API.

The runtime exposes one global, `nsmbu`.
It holds the lifecycle fields, the log and status helpers, and one field per module.

```luau
nsmbu.guest.read_u32(0x105E2C80)
nsmbu.level.set_time(999)
nsmbu.player(0)
```

Luau type definitions for editors live in [../../modding/sdk/nsmbu.d.luau](../../modding/sdk/nsmbu.d.luau).
The definitions are editor-only; the runtime builds the same table.
The runtime wiring that builds that table lives in [../../runtime/src/mods/sdk/nsmbu.cpp](../../runtime/src/mods/sdk/nsmbu.cpp) and mirrors the declaration module by module.

`require("nsmbu")` and `require("nsmbu.*")` still resolve to the same tables, but new mods use the global.
Names that start with `nsmbu` are reserved for the runtime.
Each package has its own Luau state, so one package cannot replace another package's global.

| Module | Role | Status |
| --- | --- | --- |
| `nsmbu` | Log, status, time, package | Live |
| `nsmbu.config` | Package option reads | Live |
| `nsmbu.guest` | Guest memory read/write (heap alloc later) | Live (R/W) |
| `nsmbu.level` | Course timer, course coins, score, live actors, spawn | Live |
| `nsmbu.player` | Player handles and iterator | Live |
| `nsmbu.scene` | Boot flow phase probes and scene transitions | Live |
| `nsmbu.fn` | Generated function addresses | Proposal (empty table) |
| `nsmbu.hooks` | Lifecycle callbacks (live); hook, replace, original, call (proposal) | Partial |
| `nsmbu.input` | Pad snapshot, button helpers | Proposal (`press` injection is live) |
| `nsmbu.data` | Per-package data files | Proposal (empty table) |
| `nsmbu.enemy` / `nsmbu.boss` / `nsmbu.blocks` / `nsmbu.env` | Future game domains | Stub empty tables |

## Rules

- Call these functions on the game thread only.
- A host function returns `nil` and a string message on failure.
- It does not raise a Luau error for a bad address, a missing file, or a full heap.
- A raised error inside a callback unloads that package only.
- Integer registers and guest integers are in the range `0` .. `4294967295`, except the signed read helpers.
- Float registers are Luau numbers.
- Guest multi-byte values are big-endian.
- One guest read or write is at most 1 MiB.
- Strings passed to the host are copied before the call returns.
- Do not keep a guest pointer across frames unless you allocated it with `nsmbu.guest.alloc` (proposal).

## Lifecycle

Implemented in phase 2.

Assign one function per event on `nsmbu.hooks`.
Omit a field when the package does not need that event.

```luau
function nsmbu.hooks.on_logic_step(step)
end

function nsmbu.hooks.on_config_changed()
end

function nsmbu.hooks.on_loaded()
end

function nsmbu.hooks.on_unload()
end
```

| Field | Type | When it runs |
| --- | --- | --- |
| `on_logic_step` | `(step: number) -> ()` | Once per original logic step, after actor execution. Interpolated draws do not call it. `step` is the full logic-step counter. |
| `on_config_changed` | `() -> ()` | After the player edits an option for this package. |
| `on_loaded` | `() -> ()` | Once right after the entry script returns, before the first logic step. |
| `on_unload` | `() -> ()` | When the package is disabled or the profile changes. Process exit does not promise this call. |

Package order follows manifest dependencies.

## Package

Implemented in phase 2.

`nsmbu.package` is read-only.

| Field | Type | Meaning |
| --- | --- | --- |
| `id` | string | Manifest id. |
| `version` | string | Manifest version. |
| `path` | string | Absolute directory of the installed package. |

Do not write these fields.
The next host call replaces the table if a script overwrites it.

## Log and status

Implemented in phase 2.

```luau
nsmbu.log("ready")
nsmbu.log_int("coins", 12)
nsmbu.log_hex("addr", 0x105E2C80)
nsmbu.log_float("dt", nsmbu.logic_dt())
nsmbu.status("Coins: 12")
```

| Function | Result |
| --- | --- |
| `log(message)` | Writes one line to the game log, tagged with the package id. Returns `true`. |
| `log_int(label, value)` | Same, with a decimal integer. |
| `log_hex(label, value)` | Same, with an 8-digit hex integer. |
| `log_float(label, value)` | Same, with a number. |
| `status(message)` | Sets the Mods tab status line for this package. The host keeps at most 1024 bytes. Returns `true`. |

`label` and `message` must be strings.
A bad type returns `nil` and a message.

## Time

Implemented in phase 2.

```luau
local step = nsmbu.logic_step()
local seconds = nsmbu.logic_dt()
```

| Function | Result |
| --- | --- |
| `logic_step()` | The same counter passed to logic-step callbacks. |
| `logic_dt()` | Length of the current logic step in seconds. |

## Config

Implemented in phase 2.

Values come from the Mods tab.
They match the manifest `options` entry.

```luau
local name = nsmbu.config.string("label", "Coins")
local every = nsmbu.config.number("poll_every", 30)
local on = nsmbu.config.bool("enabled", true)
```

| Function | Result |
| --- | --- |
| `nsmbu.config.string(id, fallback)` | The string option, or `fallback` when the id is missing or not a string. Enum options return the selected choice name. |
| `nsmbu.config.number(id, fallback)` | The number, or `fallback` when the id is missing or not a number. |
| `nsmbu.config.bool(id, fallback)` | The boolean, or `fallback` when the id is missing or not a boolean. |

`id` is the option id, not the display name.
`fallback` is required.
A wrong `fallback` type returns `nil` and a message.

Option edits are visible on the next `nsmbu.config.*` call.
They also fire `config_changed` callbacks.
A package with `content_dir` still restarts before new files apply.
The script options themselves do not wait for that restart.

## Guest memory

Implemented in phase 2 (reads and writes).
Guest heap alloc is still proposal.

```luau
local value = nsmbu.guest.read_u32(0x105E2C80)
nsmbu.guest.write_u32(0x105E2C80, value + 1)

local bytes = nsmbu.guest.read(0x105E2C80, 16)
nsmbu.guest.write(0x105E2C80, bytes)
```

Reads and writes cover MEM1, MEM2, and the foreground bucket.
An address outside those ranges returns `nil` and `"address out of range"`.
A size above 1 MiB returns `nil` and `"size too large"`.

| Function | Result |
| --- | --- |
| `read_u8(addr)` | Integer `0` .. `255`. |
| `read_s8(addr)` | Integer `-128` .. `127`. |
| `read_u16(addr)` | Integer `0` .. `65535`. Big-endian. |
| `read_s16(addr)` | Integer `-32768` .. `32767`. Big-endian. |
| `read_u32(addr)` | Integer `0` .. `4294967295`. Big-endian. |
| `read_s32(addr)` | Signed 32-bit integer. Big-endian. |
| `read_f32(addr)` | Luau number. Big-endian IEEE-754 binary32. |
| `read_f64(addr)` | Luau number. Big-endian IEEE-754 binary64. |
| `write_u8(addr, value)` | `true` on success. The value is truncated to 8 bits. |
| `write_s8(addr, value)` | `true`. Truncated to 8 bits. |
| `write_u16` / `write_s16` | `true`. Truncated to 16 bits. Big-endian. |
| `write_u32` / `write_s32` | `true`. Truncated to 32 bits. Big-endian. |
| `write_f32` / `write_f64` | `true`. Big-endian. |
| `read(addr, size)` | A Luau string of `size` bytes. |
| `write(addr, data)` | `true`. `data` is a Luau string. |

Alignment is not required.
A partial object that crosses a valid region boundary fails the whole call.

### Guest heap

Proposal (not in phase 2).

Use this only when game code must hold a pointer to bytes you own.
Luau tables do not need it.

```luau
local block = nsmbu.guest.alloc(256)
if block then
  nsmbu.guest.write_u32(block, 1)
  nsmbu.guest.free(block)
end
```

| Function | Result |
| --- | --- |
| `alloc(size)` | A 16-byte-aligned guest address, or `nil` and `"out of memory"`. |
| `free(addr)` | `true`. `free(0)` is `true`. A pointer this package did not allocate returns `nil` and a message. |

The default heap is 256 KiB.
`luau.heap_size` in the manifest raises it, up to 8 MiB.
`alloc(0)` returns `nil` and a message.
The heap is per package.
Save states store it with that package id when this function exists.

## Level and players

Implemented in phase 2.

These helpers wrap USA v1.3.0 guest layout for common course scripts.
Prefer them over raw `nsmbu.guest` reads when they cover the job.
Addresses stay inside the host.

```luau
if nsmbu.level.active() then
  nsmbu.level.set_time(999)
  nsmbu.level.set_coins(0)
  nsmbu.level.set_score(0)
end

local p = nsmbu.player(0)
if p and p:entered() then
  p:set_lives(99)
  p:set_mode(p:mode())
  local pos = p:pos()
  if pos and p:grounded() then
    p:set_pos(pos.x, pos.y, pos.z)
  end
end

for p in nsmbu.player.players() do
  local lives = p:lives()
end
```

### `nsmbu.level`

| Function | Result |
| --- | --- |
| `active()` | `true` when the course timer singleton is present. |
| `time()` | Remaining time in whole seconds, or `nil` and a message. |
| `set_time(seconds)` | Writes the timer. Values clamp to `0` .. `999`. |
| `add_time(seconds)` | Adds (or subtracts) seconds, then clamps. |
| `coins()` / `set_coins(n)` | Course-wide coin total from `FieldGame`. |
| `score()` / `set_score(n)` | Course score from `FieldGame`. |

### `nsmbu.player`

The module is callable: `nsmbu.player(index)` is the same as `nsmbu.player.get(index)`.
`nsmbu.player.players()` returns an iterator over entered players only.

`nsmbu.player(index)` / `nsmbu.player.get(index)` returns a player handle for `index` `0` .. `3`, or `nil` when `FieldGame` is missing or the index is bad.

| Method | Result |
| --- | --- |
| `entered()` | `true` when that slot has an entered player. |
| `lives()` / `set_lives(n)` | Field lives. Fails with `"not entered"` when the slot is empty. |
| `mode()` / `set_mode(n)` | Power-up mode `0` .. `8`. `mode` prefers the actor value when it is valid. `set_mode` writes field data and the actor when present. |
| `coins()` / `set_coins(n)` | Field coin count for that player. |
| `pos()` | Actor position as `{ x, y, z }`. Fails with `"no player actor"` when the actor is missing. |
| `set_pos(x, y, z)` | Writes actor position. You may also pass one table `{ x, y, z }`. |
| `speed()` | Actor speed as `{ x, y, z }`. Fails with `"no player actor"` when the actor is missing. |
| `set_speed(x, y, z)` | Writes actor speed. You may also pass one table `{ x, y, z }`. |
| `speed_y()` / `set_speed_y(v)` | Actor vertical speed only. Same actor requirement as `speed`. |
| `grounded()` | `true` when the actor foot collision reports ground. Fails with `"no player actor"` when the actor is missing. |
| `actor()` | Guest address of the player actor, or `nil` and a message. |

`p.index` is the slot number.
Do not keep a player handle across frames and treat it as a stable guest pointer.
Call `nsmbu.player` / `nsmbu.player.get` or `nsmbu.player.players` again each logic step.

### Level actors and spawn

Live course actors, actor handles, and spawning.
These wrap `ActorMgr`, `ActorBase`, and `Profile` from the USA v1.3.0 layout.

```luau
for _, a in nsmbu.level.actors() do
  local pid = a:profile_id()
  if pid == 467 then
    local pos = a:pos()
  end
end

nsmbu.level.spawn(467, 100, 0, 0, { layer = 0, param0 = 0 })
```

| Function | Result |
| --- | --- |
| `actors()` | Array of actor handles for the live actor cache. The array holds every live actor, including menu actors outside `pos` use. |
| `count(profile_id?)` | Number of live actors, or of one profile when passed. |
| `spawn(profile_id, x, y, z, options?)` | Writes an `ActorCreateParam` and calls `ActorMgr::createLater`. Returns `true`. The actor appears on the next logic step, not in the same step. `options` may set `param0`, `param1` (both `0`), and `layer` (`0`) |

Spawns run on the game thread only.
A course must be running; otherwise the call fails.
An unknown or unregistered profile id fails with `"unknown profile"`.
Profile numbers come from the game's profile list:
coin `395`, goomba `467`, mushroom `591`, fire flower `592`, one-up `597`, star `853`.
`param0` / `param1` are the level designer nybbles ("spritedata").

Each handle is a table with `addr` (the guest address) and these methods:

| Method | Result |
| --- | --- |
| `address()` | Guest address of the `ActorBase`. |
| `id()` | `ActorUniqueID` value. |
| `profile_id()` | Profile id of the actor. |
| `is_actor()` | `true` when the runtime type derives from `Actor` (a course actor with position, speed, and scale). Menu and background actors report `false`. |
| `is_map_actor()` | `true` when the actor was spawned from level data. |
| `active()` | `true` when the create phase completed. |
| `layer()` / `param0()` / `param1()` / `player_no()` | Raw actor fields. |
| `pos()` / `set_pos(x, y, z)` | Position. You may pass one table `{ x, y, z }`. Requires `is_actor()`. |
| `speed()` / `set_speed(x, y, z)` | Speed. Same actor requirement as `pos`. |
| `scale()` / `set_scale(x, y, z)` | Scale. Same actor requirement as `pos`. |
| `delete()` | Sets the game's delete request flag. The game removes the actor on its next delete pass. Works on every actor handle. |

Do not keep an actor handle across logic steps.
The game reuses actor slots and memory, so a stale handle can point at a different actor.
Re-read `nsmbu.level.actors()` each step, and call `profile_id()` before you edit or delete a handle.

## Scene flow

Implemented in phase 2 extended with the title-skip groundwork.

The boot flow runs the menus, the save select, and the world map as sead tasks.
`nsmbu.scene` reads the current phase and requests transitions the same way the
game's own menus do.

| Function | Result |
| --- | --- |
| `phase()` | The current boot-flow task class id, or `nil` and a message before the scene exists. |
| `title()` | `true` when the title layout phase is current. |
| `file_select()` | `true` when the small file-select task is the current scene. |
| `save_select()` | `true` when the current scene factory is `0x024C11F4` (`ChallengeSelectScene`). |
| `course_task()` | `true` when the `CourseTask` scene is current. |
| `file_index()` | The save file index after the save select has applied a file. `nil` and a message before that. |
| `flow_phase()` | `true` when the application flow phase is current. |
| `map_phase()` | `true` when the world map gamescene phase is current. |
| `course()` | `true` when the game scene task (the animated map behind the menus, the world map, and course scenes) is alive. |
| `course_select()` | `true` when the world map select task is alive; the world map is up and interactive. |
| `open_title()` | Requests a scene change into the title task. The game continues its own flow. Returns `nil` and `"scene busy"` while a scene change is queued or the fader is busy. |
| `open_first_save()` | Writes `0` to `SaveData.header.last_file` on `SaveMgr` (`0x101E7DA4`), applies save file 0 with guest `0x029D7648`, then calls guest `0x029872BC`. That function requests `CourseSelectTask` (`0x0202B86C`) when save file 0 has a record, and factory `0x0202B570` when the file is empty. After the call, `MainGame.mIsSinglePlayerDRC` (`0x101E7D7C` + `0x14`) is `1` for the GamePad and `0` for the Pro Controller, so the map reads the same device as the host. Returns `nil` and `"scene busy"` or `"file not ready"` when it cannot run. Returns true after the call. |
| `open_file_select()` | Selects save file 0 through `SubjectMgr::selectSaveFile` on an existing menu task, then enters menu state `0x1022C0E4` (`0x0297D268`). It does not create the menu factory and does not enter the challenge-select state (`0x0297D58C`). Returns `nil` and `"scene busy"`, `"menu missing"`, `"state not ready"`, or `"file not ready"` when it cannot run. Returns true when the file select runs. |
| `open_course_select()` | Requests the world map select task. Returns `nil` and `"scene busy"` while a scene change is queued or the fader is busy. |
| `confirm_file()` | Confirms the save select. The routine sets the file and story gates on the flow object and tells the app to confirm, the way the save select does after a file is chosen. Returns `nil` and `"scene busy"` while a scene change is queued or the fader is busy, and does not write the gates in that case. |
| `load_story(world_no, course_no)` | Sets the next story location on `CourseInfo` and requests the game scene task, the way the save select does. `world_no` is 0-indexed (world 1 = 0). |

The transitions call `Scene::setNextScene` and `Scene::setCourse_Story` inside the
port; mods never pass guest registers or addresses.
A request only moves the flow it lands on, so call transitions from the matching
phase or after the flow has reached the map.
Call the same transition again on a later logic step when it returns `"scene busy"`.
`Scene::setNextScene` accepts the request after the queued change clears and the fader is idle.

## Function addresses

Proposal (phase 4 with hooks).

```luau
local addr = nsmbu.fn.dScnPly_Execute
```

`nsmbu.fn` is a generated table.
Keys are public function names from the USA headers.
Values are integer addresses for the running build.
The European build map translates them at load.
A name that does not exist is `nil`.

Do not write `fn`.
The runtime does not accept a raw USA address on a European build unless that address is in the map.
Prefer `fn`.

There is no guest address for a Luau function.
The game cannot call a Luau function through a function pointer.

## Hooks

Proposal (phase 4).

```luau
local id = nsmbu.hooks.hook(nsmbu.fn.dScnPly_Execute, "entry", function(ctx)
  nsmbu.log_hex("r3", ctx.r3)
end)

nsmbu.hooks.hook(nsmbu.fn.dScnPly_Execute, "return", function(ctx)
  ctx.r3 = 0
end)

nsmbu.hooks.unhook(id)
```

| Function | Result |
| --- | --- |
| `hook(target, when, callback)` | An integer id. `when` is `"entry"` or `"return"`. |
| `replace(target, callback)` | An integer id. |
| `unhook(id)` | `true`. Removes a hook or a replacement owned by this package. |

`target` is an address from `nsmbu.fn` or a mapped integer.
`callback` is `function(ctx)`.
A second package that `replace`s the same function fails at enable time.
Any number of entry and return hooks is allowed.
Entry hooks run in package order, then registration order.
The replacement runs after the entry hooks, or the game body runs when there is no replacement.
Return hooks then run in reverse order.

The port hooks in `hooks*.txt` stay outside this sequence.
Frame interpolation decides whether the function runs at all.

### `ctx`

`ctx` is a table.
The runtime fills it before the callback and copies it back after the callback returns.

| Field | Meaning |
| --- | --- |
| `address` | Guest address of the hooked function. Read-only. The copy-back ignores writes. |
| `r3` .. `r10` | Integer argument registers. |
| `f1` .. `f8` | Float argument registers. |
| `skip` | Entry hooks only. Set `true` to skip the game body, the replacement, and later entry hooks. Return hooks still run. |

A return hook can change `r3`, `r4`, and `f1` to change the value the caller sees.
Other register writes from a return hook are copied back too.
The game ABI uses `r3` and `r4` for a two-register return, and `f1` for a float return.

An entry hook that only reads `ctx` must not set `skip`.

### `original`

```luau
nsmbu.hooks.replace(nsmbu.fn.cLib_addCalc2, function(ctx)
  nsmbu.hooks.original()
  nsmbu.log_int("result", ctx.r3)
end)
```

| Function | Result |
| --- | --- |
| `original()` | Runs the game body once. Returns `true`. After it returns, `ctx` holds the registers the game body left. |

`original` is valid only inside a replacement callback.
A second call in the same invocation returns `nil` and `"original already called"`.
A replacement that never calls `original` skips the game body.
Entry and return hooks cannot call `original`.

## Calls

Proposal (phase 4).

```luau
local ok, err = nsmbu.hooks.call(nsmbu.fn.cLib_addCalc2, {
  r3 = object,
  f1 = 1.0,
})
```

| Function | Result |
| --- | --- |
| `call(target, args)` | `true`, or `nil` and a message. `args` is an optional table of `r3` .. `r10` and `f1` .. `f8`. Omitted registers are `0`. |

The return registers are not returned as a second table.
Read them with a return hook, or with `original` inside a replacement.
`call` is for a script that wants to invoke a game function from a logic-step callback or from a hook.

```luau
local result = nsmbu.hooks.call_result(nsmbu.fn.cLib_addCalc2, { r3 = object, f1 = 1.0 })
if result then
  nsmbu.log_int("r3", result.r3)
end
```

| Function | Result |
| --- | --- |
| `call_result(target, args)` | A table `{ r3, r4, f1 }`, or `nil` and a message. |

`call_result` is the form to use from a logic-step callback.
Both functions refuse a target that is not a translated game function.
Both refuse a call that is already on the stack for the same package beyond 8 nested `call` / `call_result` frames.

Do not use `call` on a function that runs thousands of times per frame.
Replace that function instead.

## Input

`press` is live. The pad snapshot and button read helpers stay proposal.

```luau
nsmbu.input.press("A", 3)
nsmbu.input.press({ "Down", "Down" }, 2)
```

| Function | Result |
| --- | --- |
| `press(buttons, steps)` | Injects those buttons into the pad for the next `steps` logic steps. `buttons` is one name or a mixed list of names (`"A"`, `"Down"`, `"Plus"`, ...). Returns `true`, or `nil` and a message for a bad type. Presses stack with host input; they end on their own. |


```luau
local pad = nsmbu.input()
if pad.buttons & nsmbu.input.button.A ~= 0 then
  nsmbu.log("A")
end
```

The `nsmbu.input` module is callable.
`nsmbu.input()` returns this table.
It does not change the pad.

| Field | Type | Meaning |
| --- | --- | --- |
| `buttons` | integer | Bit mask. The bits match `runtime/src/input.h`. |
| `lx`, `ly` | number | Left stick, about `-1` .. `1`. |
| `rx`, `ry` | number | Right stick. |
| `touch` | boolean | Touch is down. |
| `tx`, `ty` | number | Touch position. |

`nsmbu.input.button` holds the mask constants.

| Name | Value |
| --- | --- |
| `A` | `0x8000` |
| `B` | `0x4000` |
| `X` | `0x2000` |
| `Y` | `0x1000` |
| `L` | `0x0020` |
| `R` | `0x0010` |
| `ZL` | `0x0080` |
| `ZR` | `0x0040` |
| `Plus` | `0x0008` |
| `Minus` | `0x0004` |
| `Home` | `0x0002` |
| `Up` | `0x0200` |
| `Down` | `0x0100` |
| `Left` | `0x0800` |
| `Right` | `0x0400` |
| `StickL` | `0x00040000` |
| `StickR` | `0x00020000` |

```luau
local down = nsmbu.input.button.held("A")
```

| Function | Result |
| --- | --- |
| `nsmbu.input.button.held(name)` | `true` when that button is down. `name` is a key of `nsmbu.input.button` such as `"A"`. A bad name returns `nil` and a message. |
| `nsmbu.input.button.pressed(name)` | `true` when the button is down now and was up on the previous `nsmbu.input()` or `nsmbu.input.button.pressed` sample in this package. |

This API does not inject buttons.
It does not rumble the pad.

## Data files

Proposal.

Files live in `mod-data/<package id>/` under the host config directory.
The name is one path segment.
`/`, `\`, `..`, and an empty name fail.
The host rejects symlinks and Windows device names.
One call moves at most 1 MiB.

```luau
nsmbu.data.write("counter.txt", "3")
local text = nsmbu.data.read("counter.txt")
nsmbu.data.remove("counter.txt")
```

| Function | Result |
| --- | --- |
| `read(name)` | The file bytes as a string, or `nil` and a message when the file is missing. |
| `write(name, data)` | The number of bytes written. The write replaces the whole file. |
| `remove(name)` | `true`. A missing file is `true`. |

`data` is a Luau string and may contain zero bytes.

## Content files

Live (phase 3).

Scripts do not register file replacements.
Put files under `content_dir` in the manifest.
The map is fixed until the next process start.
A package with `content_dir` needs a restart; a script-only package does not.
See [luau-mods.md](luau-mods.md) and [content.md](content.md).

There is no `nsmbu.content` module in API v1.

## Standard library

| Available | Absent |
| --- | --- |
| `string`, `table`, `math`, `coroutine`, `bit32`, `utf8` | `io`, `os`, `debug`, `package.loadlib`, FFI |
| `require` of host `nsmbu` / `nsmbu.*` modules | `require` of an unknown `nsmbu.*` name |
| `require` of another `.luau` file in this package | `require` of a path with `..` or an absolute path |
| Host modules as described here | A native library, a PowerPC ELF, a Cemu `rules.txt` |

`coroutine` may yield only inside the callback that resumed it.
A yield across the host boundary returns `nil` and `"yield across host call"` from the host function that notices it.
Prefer no coroutines in hooks.

## Examples

A status line from an option and player coins (phase 2):

```luau
function nsmbu.hooks.on_logic_step(step)
  if step % nsmbu.config.number("every", 30) ~= 0 then
    return
  end
  local p = nsmbu.player(0)
  if not p or not p:entered() then
    return
  end
  local coins, err = p:coins()
  if not coins then
    nsmbu.log(err)
    return
  end
  nsmbu.status(nsmbu.config.string("label", "Coins") .. ": " .. coins)
end
```

Count calls to a play-scene function, then leave the game body unchanged (proposal until phase 4):

```luau
local calls = 0

nsmbu.hooks.hook(nsmbu.fn.dScnPly_Execute, "entry", function(ctx)
  calls = calls + 1
end)

nsmbu.hooks.hook(nsmbu.fn.dScnPly_Execute, "return", function(ctx)
  if calls % 60 == 0 then
    nsmbu.log_int("play scene calls", calls)
  end
end)
```

Save a counter beside the package (proposal until data files land):

```luau
function nsmbu.hooks.on_unload()
  nsmbu.data.write("calls.txt", tostring(calls))
end
```

## Not in version 1

- Drawing text or images (the HUD service from SDK v2).
- Button injection, rumble, or touch injection.
- Audio playback.
- Changing the content file map without a restart.
- A guest function pointer that enters Luau.
- Loading a `.so`, `.dll`, or `.dylib`.
- Shader or resolution overrides. Those stay in Cemu packages.
- Named C parameters generated from header signatures. Hooks use registers.
- Filled enemy, boss, blocks, and env helpers (modules exist as empty stubs).

## Errors

| Message | Cause |
| --- | --- |
| `address out of range` | Guest address is outside MEM1, MEM2, and the foreground bucket. |
| `size too large` | Read or write is above 1 MiB, or the heap request is above the package cap. |
| `out of memory` | The package guest heap is full. |
| `bad type` | A host function received the wrong Luau type. |
| `unknown option` | Not used. Config returns `fallback` instead. |
| `unknown function` | `nsmbu.fn` name or `call` target is not a game function on this build. |
| `already replaced` | This function already has a replacement. Enable fails before the script runs when another package owns it. |
| `original already called` | `original` ran twice in one replacement. |
| `original outside replacement` | `original` ran from an entry hook, a return hook, or a logic-step callback. |
| `bad file name` | Data file name is not one flat segment. |
| `yield across host call` | A coroutine yielded through a host function. |
