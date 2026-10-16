# Modding

| Path | Contents |
| --- | --- |
| [sdk/](sdk/) | Luau type definitions for the global `nsmbu` API, one file per module (`nsmbu.d.luau` declares the global) |
| [examples/](examples/) | Example Luau packages for the mod manager |

Format Luau with [StyLua](https://github.com/JohnnyMorganz/StyLua) (`cargo install stylua --features luau`).
`just lint` formats then type-checks the examples.

```bash
stylua --config-path modding/.stylua.toml modding/examples
```

Config: [`.stylua.toml`](.stylua.toml).

Docs: [docs/modding/](../docs/modding/).
