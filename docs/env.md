# Runtime environment variables

`runtime/src/env.h` is the list of `NSMBU_*` names the runtime reads.
Each name is an `inline` `env::Var` in that header.
`runtime/src/env.cpp` implements `env::unset`.

A read is a call on the object.
`env::NO_HOST_INPUT()` returns the same pointer `getenv` returns.
`env::NO_HOST_INPUT.name()` is the string `"NSMBU_NO_HOST_INPUT"` when a log or a `setenv` needs it.
`integer_or`, `real_or`, `u64_or`, and `ul_or` return the parsed number, or the fallback when the variable is unset.
`enabled` is true when the value is set and `atoi` is not `0`.
`present` is true when the variable is set.

The C++ name is the environment name without `NSMBU_`.
`NSMBU_VK_DRAW_BATCH` is `env::VK_DRAW_BATCH`.

To add a variable, add one `ENV` line in `runtime/src/env.h`.

A write stays `setenv` at the call site (`_putenv_s` on Windows).
An `NSMBU_*` write passes `.name()`, as in `setenv(env::STATE_DIR.name(), path, 0)`.
`env::unset(env::RENDERER_RUNTIME)` clears a catalog variable on every platform.

Host names stay on `getenv`: `HOME`, `APPDATA`, `USER`, `USERNAME`, `USERPROFILE`, and `XDG_CONFIG_HOME`.
The MoltenVK settings `MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS` and `MVK_CONFIG_PREFILL_METAL_COMMAND_BUFFERS` stay on `setenv` in the Vulkan backend.
Those names are not in `runtime/src/env.h`.

On Android, `env.txt` lines are file data applied with `setenv`.
They are not a second list of variable names.

What a variable does stays in the feature doc that already describes it (`docs/automation.md`, `docs/vulkan.md`, and the other feature pages).
This file is how a variable is defined in code, not a catalog of gameplay effects.

The host test `env` in `runtime/tools/env_test.cpp` covers `.name()`, a read after `setenv`, and `env::unset`.
