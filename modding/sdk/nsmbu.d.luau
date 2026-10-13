type GuestAddr = number
type ListenId = number

type LogicStepCallback = (step: number) -> ()
type VoidCallback = () -> ()

type Package = {
	id: string,
	version: string,
	path: string,
}

type Config = {
	string: (id: string, fallback: string) -> (string?, string?),
	number: (id: string, fallback: number) -> (number?, string?),
	bool: (id: string, fallback: boolean) -> (boolean?, string?),
}

type Guest = {
	read_u8: (addr: GuestAddr) -> (number?, string?),
	read_s8: (addr: GuestAddr) -> (number?, string?),
	read_u16: (addr: GuestAddr) -> (number?, string?),
	read_s16: (addr: GuestAddr) -> (number?, string?),
	read_u32: (addr: GuestAddr) -> (number?, string?),
	read_s32: (addr: GuestAddr) -> (number?, string?),
	read_f32: (addr: GuestAddr) -> (number?, string?),
	read_f64: (addr: GuestAddr) -> (number?, string?),
	write_u8: (addr: GuestAddr, value: number) -> (boolean?, string?),
	write_s8: (addr: GuestAddr, value: number) -> (boolean?, string?),
	write_u16: (addr: GuestAddr, value: number) -> (boolean?, string?),
	write_s16: (addr: GuestAddr, value: number) -> (boolean?, string?),
	write_u32: (addr: GuestAddr, value: number) -> (boolean?, string?),
	write_s32: (addr: GuestAddr, value: number) -> (boolean?, string?),
	write_f32: (addr: GuestAddr, value: number) -> (boolean?, string?),
	write_f64: (addr: GuestAddr, value: number) -> (boolean?, string?),
	read: (addr: GuestAddr, size: number) -> (string?, string?),
	write: (addr: GuestAddr, data: string) -> (boolean?, string?),
}

type Fn = {
	[string]: GuestAddr?,
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
}

type Scene = {
	title: () -> boolean,
	file_select: () -> boolean,
	flow_phase: () -> boolean,
	map_phase: () -> boolean,
	phase: () -> (number?, string?),
	course_select: () -> boolean,
	course: () -> boolean,
	open_title: () -> (boolean?, string?),
	open_file_select: () -> (boolean?, string?),
	open_course_select: () -> (boolean?, string?),
	confirm_file: () -> (boolean?, string?),
	load_story: (world_no: number, course_no: number) -> (boolean?, string?),
}

type ButtonName = "A" | "B" | "X" | "Y"
	| "L" | "R" | "ZL" | "ZR"
	| "Plus" | "Minus" | "Home"
	| "Up" | "Down" | "Left" | "Right"
	| "StickL" | "StickR"

type Pad = {
	buttons: number,
	lx: number,
	ly: number,
	rx: number,
	ry: number,
	touch: boolean,
	tx: number,
	ty: number,
}

type Button = {
	A: number,
	B: number,
	X: number,
	Y: number,
	L: number,
	R: number,
	ZL: number,
	ZR: number,
	Plus: number,
	Minus: number,
	Home: number,
	Up: number,
	Down: number,
	Left: number,
	Right: number,
	StickL: number,
	StickR: number,

	held: (name: ButtonName) -> (boolean?, string?),
	pressed: (name: ButtonName) -> (boolean?, string?),
}

type Input = (() -> Pad) & {
	button: Button,
	press: (buttons: string | { string }, steps: number) -> (boolean?, string?),
}

type Data = {
	read: (name: string) -> (string?, string?),
	write: (name: string, data: string) -> (number?, string?),
	remove: (name: string) -> (boolean?, string?),
}

type SpawnOptions = {
	param0: number?,
	param1: number?,
	layer: number?,
}

type Level = {
	active: () -> boolean,
	time: () -> (number?, string?),
	set_time: (seconds: number) -> (boolean?, string?),
	add_time: (seconds: number) -> (boolean?, string?),
	coins: () -> (number?, string?),
	set_coins: (coins: number) -> (boolean?, string?),
	score: () -> (number?, string?),
	set_score: (score: number) -> (boolean?, string?),
	actors: () -> { Actor },
	count: (profile_id: number?) -> (number?, string?),
	spawn: (profile_id: number, x: number, y: number, z: number, options: SpawnOptions?) -> (boolean?, string?),
}

type Vec3 = {
	x: number,
	y: number,
	z: number,
}

type Actor = {
	addr: number,

	address: (self: Actor) -> (number?, string?),
	id: (self: Actor) -> (number?, string?),
	profile_id: (self: Actor) -> (number?, string?),
	is_actor: (self: Actor) -> (boolean?, string?),
	is_map_actor: (self: Actor) -> (boolean?, string?),
	active: (self: Actor) -> (boolean?, string?),
	layer: (self: Actor) -> (number?, string?),
	player_no: (self: Actor) -> (number?, string?),
	param0: (self: Actor) -> (number?, string?),
	param1: (self: Actor) -> (number?, string?),
	pos: (self: Actor) -> (Vec3?, string?),
	set_pos: (self: Actor, x: number | Vec3, y: number?, z: number?) -> (boolean?, string?),
	speed: (self: Actor) -> (Vec3?, string?),
	set_speed: (self: Actor, x: number | Vec3, y: number?, z: number?) -> (boolean?, string?),
	scale: (self: Actor) -> (Vec3?, string?),
	set_scale: (self: Actor, x: number | Vec3, y: number?, z: number?) -> (boolean?, string?),
	delete: (self: Actor) -> (boolean?, string?),
}

type Player = {
	index: number,

	entered: (self: Player) -> (boolean?, string?),
	lives: (self: Player) -> (number?, string?),
	set_lives: (self: Player, lives: number) -> (boolean?, string?),
	mode: (self: Player) -> (number?, string?),
	set_mode: (self: Player, mode: number) -> (boolean?, string?),
	coins: (self: Player) -> (number?, string?),
	set_coins: (self: Player, coins: number) -> (boolean?, string?),
	speed_y: (self: Player) -> (number?, string?),
	set_speed_y: (self: Player, speed: number) -> (boolean?, string?),
	pos: (self: Player) -> (Vec3?, string?),
	set_pos: (self: Player, x: number | Vec3, y: number?, z: number?) -> (boolean?, string?),
	speed: (self: Player) -> (Vec3?, string?),
	set_speed: (self: Player, x: number | Vec3, y: number?, z: number?) -> (boolean?, string?),
	grounded: (self: Player) -> (boolean?, string?),
	actor: (self: Player) -> (number?, string?),
}

type Players = {
	get_player: (index: number) -> (Player?, string?),
	get_players: () -> { Player },
}

type Empty = {}

declare nsmbu: {
	package: Package,

	log: (message: string) -> (boolean?, string?),
	log_int: (label: string, value: number) -> (boolean?, string?),
	log_hex: (label: string, value: number) -> (boolean?, string?),
	log_float: (label: string, value: number) -> (boolean?, string?),
	status: (message: string) -> (boolean?, string?),
	logic_step: () -> number,
	logic_dt: () -> number,
	on_logic_step: (callback: LogicStepCallback) -> (ListenId?, string?),
	on_config_changed: (callback: VoidCallback) -> (ListenId?, string?),
	on_loaded: (callback: VoidCallback) -> (ListenId?, string?),
	on_unload: (callback: VoidCallback) -> (ListenId?, string?),
	unlisten: (id: ListenId) -> (boolean?, string?),

	config: Config,
	guest: Guest,
	fn: Fn,
	hooks: Empty,
	input: Input,
	data: Data,
	enemy: Empty,
	boss: Empty,
	blocks: Empty,
	env: Empty,
	level: Level,
	player: Players,
	scene: Scene,
}
