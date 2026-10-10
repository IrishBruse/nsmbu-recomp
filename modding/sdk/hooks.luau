export type GuestAddr = number
export type HookId = number
export type HookWhen = "entry" | "return"

export type HookCtx = {
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

export type HookCallback = (ctx: HookCtx) -> ()

export type CallArgs = {
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

export type CallResult = {
	r3: number,
	r4: number,
	f1: number,
}

export type Module = {
	hook: (target: GuestAddr, when: HookWhen, callback: HookCallback) -> (HookId?, string?),
	replace: (target: GuestAddr, callback: HookCallback) -> (HookId?, string?),
	unhook: (id: HookId) -> (boolean?, string?),
	original: () -> (boolean?, string?),
	call: (target: GuestAddr, args: CallArgs?) -> (boolean?, string?),
	call_result: (target: GuestAddr, args: CallArgs?) -> (CallResult?, string?),
}

return {} :: Module
