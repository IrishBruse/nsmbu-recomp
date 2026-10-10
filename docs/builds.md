# Build maps

This port is USA only.
The title is `00050000-10101d00`.
The executable is `code/red-pro2.rpx` from USA 1.3.0 (title version 64).
Every game address in this repository (`tools/recomp/hooks*.txt`, `runtime/src`, the notes in `docs/`) is an address of that executable.
Those addresses are the *canonical ids* of the game's functions and globals.

The RPX SHA-256 is in `tools/recomp/builds.py`.
`just extract` checks `game/update/` and the merged dump against that hash.

## Regional builds

This repository ships no Europe or Japan executable map.
Language packs can still lend European or Japanese text to the USA build
([language-packs.md](language-packs.md)).

A **build map** (`tools/recomp/builds/*.json`) remains the tool to add another region later.
Derive it with `tools/recomp/mkbuildmap.py` from your own USA and regional dumps, then commit the JSON only.

```sh
python3 tools/recomp/mkbuildmap.py usa/code/red-pro2.rpx eur/code/red-pro2.rpx \
    --name EU --title 0005000010101e00 --out tools/recomp/builds/eu.json
```

## How the port uses the registry

- **The recompiler** identifies `red-pro2.rpx` by SHA-256 and names every generated function by its canonical address.
- **The runtime** uses `GC(…)` / `GD(…)` ([runtime/include/guest_addr.h](../runtime/include/guest_addr.h)).
  For the USA build the map is the identity.
- **A hooks file** may say which builds it is for (`# builds: USA`).
