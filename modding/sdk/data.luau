export type Module = {
	read: (name: string) -> (string?, string?),
	write: (name: string, data: string) -> (number?, string?),
	remove: (name: string) -> (boolean?, string?),
}

return {} :: Module
