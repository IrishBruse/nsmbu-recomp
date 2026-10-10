export type Module = {
	string: (id: string, fallback: string) -> (string?, string?),
	number: (id: string, fallback: number) -> (number?, string?),
	bool: (id: string, fallback: boolean) -> (boolean?, string?),
}

return {} :: Module
