export type ButtonName = "A" | "B" | "X" | "Y"
	| "L" | "R" | "ZL" | "ZR"
	| "Plus" | "Minus" | "Home"
	| "Up" | "Down" | "Left" | "Right"
	| "StickL" | "StickR"

export type Pad = {
	buttons: number,
	lx: number,
	ly: number,
	rx: number,
	ry: number,
	touch: boolean,
	tx: number,
	ty: number,
}

export type Button = {
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

export type Module = {
	button: Button,
} & () -> Pad

return {} :: Module
