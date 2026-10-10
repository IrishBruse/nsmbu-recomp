export type Module = {
	active: () -> boolean,
	time: () -> (number?, string?),
	set_time: (seconds: number) -> (boolean?, string?),
	add_time: (seconds: number) -> (boolean?, string?),
	coins: () -> (number?, string?),
	set_coins: (coins: number) -> (boolean?, string?),
	score: () -> (number?, string?),
	set_score: (score: number) -> (boolean?, string?),
}

return {} :: Module
