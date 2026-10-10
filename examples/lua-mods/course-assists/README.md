# Course assists

Phase 2 Lua package for common NSMBU course assists.
It uses guest memory each logic step.
It does not need hooks.

Effects when the matching option is on:

- Unlimited time writes `CourseTimer::mTime` at instance `+0x14` (singleton `0x101D15F4`).
- Unlimited lives writes `FieldPlayerData::life_cnt` at `FieldGame` instance `+0x24` plus slot `+0x4` (singleton `0x101D1604`).
- Keep power-up restores `FieldPlayerData::player_mode` and `PlayerBase::mMode` (`+0x500`) when a hit drops them.
- Jump height scale multiplies `Actor::mSpeed.y` (`+0x7C`) once at the start of each rise.

Addresses and field offsets are USA v1.3.0 from the public NSMBU headers and match `red-pro2.rpx` recomp.

Enable the package in Mods, turn on the options you want, and play a course.
Script-only packages apply live without a restart.
