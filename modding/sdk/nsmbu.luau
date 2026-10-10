export type GuestAddr = number
export type HookId = number
export type ListenId = number
export type HookWhen = "entry" | "return"
export type ListenEvent = "logic_step" | "config_changed" | "unload"

export type Package = {
	id: string,
	version: string,
	path: string,
}

export type LogicStepCallback = (step: number) -> ()
export type VoidCallback = () -> ()

export type Module = {
	package: Package,
	on_logic_step: LogicStepCallback?,
	on_config_changed: VoidCallback?,
	on_unload: VoidCallback?,

	log: (message: string) -> (boolean?, string?),
	log_int: (label: string, value: number) -> (boolean?, string?),
	log_hex: (label: string, value: number) -> (boolean?, string?),
	log_float: (label: string, value: number) -> (boolean?, string?),
	status: (message: string) -> (boolean?, string?),
	logic_step: () -> number,
	logic_dt: () -> number,
	listen: (event: ListenEvent, callback: LogicStepCallback | VoidCallback) -> (ListenId?, string?),
	unlisten: (id: ListenId) -> (boolean?, string?),
}

return {} :: Module
