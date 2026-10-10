export type GuestAddr = number

export type Module = {
	dScnPly_Execute: GuestAddr?,
	cLib_addCalc2: GuestAddr?,
	example_coin_count: GuestAddr?,
	course_timer_tick: GuestAddr?,
	player_lose_life: GuestAddr?,
	player_lose_powerup: GuestAddr?,
	player_apply_jump: GuestAddr?,
	player_actor_ptr: GuestAddr?,
	player_begin_death: GuestAddr?,
	player_clear_damage: GuestAddr?,
	course_enter: GuestAddr?,
	checkpoint_touched: GuestAddr?,
	course_quit_to_map: GuestAddr?,
} & { [string]: GuestAddr? }

return {} :: Module
