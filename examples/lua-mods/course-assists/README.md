# Course assists

This package is a hypothetical Lua version of simple NSMBU course cheats.
The loader does not run it.
The script does not contain game code or guest addresses.

People already ask for these effects on NSMBU v1.3.0:

- Unlimited time and 99 lives, requested for a Cemu graphic pack in [cemu-project/cemu_graphic_packs#752](https://github.com/cemu-project/cemu_graphic_packs/issues/752).
- Infinite time, infinite lives, a higher jump, and keeping the current suit, listed in the Cemu Cheat Engine thread [New Super Mario Bros U (CEMU)](https://fearlessrevolution.com/viewtopic.php?t=11913).

This example copies the player-facing options only.
It does not copy those cheat scripts or their addresses.

`main.lua` skips a function when the matching option is on.
`course_timer_tick` stands in for the course timer decrement.
`player_lose_life` stands in for losing a life.
`player_lose_powerup` stands in for dropping to a smaller form after a hit.
`player_apply_jump` stands in for the jump, and `ctx.f1` stands in for the vertical speed argument.

None of those names are in `nsmbu.fn` today.
Until a generated table exports them, the script logs that the symbol is missing and leaves the game alone.
A real binding must come from the USA v1.3.0 headers.
The European build map would translate those names the same way [docs/modding/lua-mods.md](../../../docs/modding/lua-mods.md) describes.
