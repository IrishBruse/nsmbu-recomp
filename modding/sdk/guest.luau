export type GuestAddr = number

export type Module = {
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

return {} :: Module
