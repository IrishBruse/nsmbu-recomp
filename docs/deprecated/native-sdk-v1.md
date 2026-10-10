# Native mod SDK v1

Status: **deprecated**.
The replacement plan is [../modding/lua-mods.md](../modding/lua-mods.md).
This ABI stays in the tree until that plan lands.
Install, enable, and the trust dialog are in [../modding/mod-manager.md](../modding/mod-manager.md).
Function hooks are the guest prototype in [mod-sdk-v2.md](mod-sdk-v2.md).

`runtime/include/nsmbu_mod.h` defines a plain C ABI.
Export `nsmbu_mod_init_v1`, validate host size and ABI, and return an initialized `NSMBUModV1`.
Initialization, configuration callbacks, game-update callbacks, and unloading run on the game thread.
The frame callback runs once per original logic step after actor execution.
Interpolated draws do not invoke it.
Host and context pointers and configuration strings remain valid until configuration changes or unload.
Copy strings if you retain them across either boundary.
Callbacks must not throw or start asynchronous guest-memory work.
Stop any owned workers before unloading.

Host services provide typed option access, a status line, logging, and bounded reads and writes of guest data RAM (MEM2, MEM1, and the foreground bucket, maximum 1 MiB per request).
Bytes use guest big-endian order.
Native packages execute trusted host code with the same permissions as the game.
The player confirms each native library once before it loads.
See Native code confirmation in the mod manager doc.
Unload callbacks run when a mod is disabled or the profile switches, before its library closes.
Process termination is not a guaranteed cleanup callback.

This ABI supports frame-driven native mods.
It does not provide arbitrary translated-function interception, PPC instruction patch execution, texture providers, or compatibility with Zelda64Recomp or BlueWake packages.

`abi_version` is `1`.
`binaries` maps a platform key to a relative library path.

Platform keys include `macos-arm64`, `macos-x86_64`, `windows-x86_64`, `windows-arm64`, `linux-x86_64`, `linux-arm64`, and `android-arm64`.
Unsupported platform binaries remain visible as incompatible.
The desktop folder and file install workflow is the current supported UI.
Android document URIs need a separate import bridge.
Online downloads and catalogues are outside v1.

A native package runs with the same permissions as the game.
Install only mods you trust.
