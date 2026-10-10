export type Vec3 = {
	x: number,
	y: number,
	z: number,
}

export type PlayerHandle = {
	index: number,

	entered: (self: PlayerHandle) -> (boolean?, string?),
	lives: (self: PlayerHandle) -> (number?, string?),
	set_lives: (self: PlayerHandle, lives: number) -> (boolean?, string?),
	mode: (self: PlayerHandle) -> (number?, string?),
	set_mode: (self: PlayerHandle, mode: number) -> (boolean?, string?),
	coins: (self: PlayerHandle) -> (number?, string?),
	set_coins: (self: PlayerHandle, coins: number) -> (boolean?, string?),
	speed_y: (self: PlayerHandle) -> (number?, string?),
	set_speed_y: (self: PlayerHandle, speed: number) -> (boolean?, string?),
	pos: (self: PlayerHandle) -> (Vec3?, string?),
	set_pos: (self: PlayerHandle, x: number | Vec3, y: number?, z: number?) -> (boolean?, string?),
	speed: (self: PlayerHandle) -> (Vec3?, string?),
	set_speed: (self: PlayerHandle, x: number | Vec3, y: number?, z: number?) -> (boolean?, string?),
	grounded: (self: PlayerHandle) -> (boolean?, string?),
	actor: (self: PlayerHandle) -> (number?, string?),
}

export type Module = {
	get: (index: number) -> (PlayerHandle?, string?),
	players: () -> (() -> PlayerHandle?),
} & (index: number) -> (PlayerHandle?, string?)

return {} :: Module
