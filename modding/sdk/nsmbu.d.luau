type GuestAddr = number
type ListenId = number
type HookId = number

type HookWhen = "entry" | "return"

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
	alloc: (size: number) -> (GuestAddr?, string?),
	free: (addr: GuestAddr) -> (boolean?, string?),
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

type HookCtx = {
	address: GuestAddr,
	r3: number,
	r4: number,
	r5: number,
	r6: number,
	r7: number,
	r8: number,
	r9: number,
	r10: number,
	f1: number,
	f2: number,
	f3: number,
	f4: number,
	f5: number,
	f6: number,
	f7: number,
	f8: number,
	skip: boolean?,
}

type HookCallback = (ctx: HookCtx) -> ()

type CallArgs = {
	r3: number?,
	r4: number?,
	r5: number?,
	r6: number?,
	r7: number?,
	r8: number?,
	r9: number?,
	r10: number?,
	f1: number?,
	f2: number?,
	f3: number?,
	f4: number?,
	f5: number?,
	f6: number?,
	f7: number?,
	f8: number?,
}

type CallResult = {
	r3: number,
	r4: number,
	f1: number,
}

type Hooks = {
	hook: (target: GuestAddr, when: HookWhen, callback: HookCallback) -> (HookId?, string?),
	replace: (target: GuestAddr, callback: HookCallback) -> (HookId?, string?),
	unhook: (id: HookId) -> (boolean?, string?),
	original: () -> (boolean?, string?),
	call: (target: GuestAddr, args: CallArgs?) -> (boolean?, string?),
	call_result: (target: GuestAddr, args: CallArgs?) -> (CallResult?, string?),
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
}

type Data = {
	read: (name: string) -> (string?, string?),
	write: (name: string, data: string) -> (number?, string?),
	remove: (name: string) -> (boolean?, string?),
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
}

type Vec3 = {
	x: number,
	y: number,
	z: number,
}

type PlayerHandle = {
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

type PlayerCall = (index: number) -> (PlayerHandle?, string?)

type Player = PlayerCall & {
	get: PlayerCall,
	players: () -> (() -> PlayerHandle?),
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
	on_unload: (callback: VoidCallback) -> (ListenId?, string?),
	unlisten: (id: ListenId) -> (boolean?, string?),

	config: Config,
	guest: Guest,
	fn: Fn,
	hooks: Hooks,
	input: Input,
	data: Data,
	enemy: Empty,
	boss: Empty,
	blocks: Empty,
	env: Empty,
	level: Level,
	player: Player,
}
