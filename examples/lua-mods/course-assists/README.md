# Course assists

Phase 2 Lua package for common NSMBU course assists.
It uses guest memory each logic step.
It does not need hooks.

Effects when the matching option is on:

- Unlimited time writes `CourseTimer::mTime` (singleton `0x101D15F4`).
- Unlimited lives writes `FieldPlayerData::life_cnt` through `FieldGame` (`0x101D1604`).
- Keep power-up restores `FieldPlayerData::player_mode` and `PlayerBase::mMode` when a hit drops them.
- Jump height scale multiplies `Actor::mSpeed.y` once at the start of each rise.

Addresses are USA v1.3.0 from the public NSMBU headers (`CourseTimer`, `FieldGame`, `PlayerMgr`, `Actor`, `PlayerBase`).

Enable the package in Mods, turn on the options you want, and play a course.
Script-only packages apply live without a restart.
