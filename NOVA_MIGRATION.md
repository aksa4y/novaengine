# Nova Engine migration

This branch starts the architectural migration from Doriax to Nova Engine.

## Current stage

- `Nova::RHI` is an API-neutral rendering boundary with a Null backend.
- `Nova::Runtime` is an explicit runtime-facing CMake target.
- `Nova::EditorCore` is an explicit editor-facing CMake target.
- Runtime does not include editor code.
- The legacy `engine/` target is deliberately retained as a compatibility layer while the source tree is migrated incrementally.

## Next mechanical step

Move the legacy runtime sources from `engine/` into `runtime/`, rename the `doriax` CMake target to `nova-runtime`, then replace the editor's direct dependency on the legacy target with `Nova::Runtime`.

## RHI strategy

Do not replace Sokol in one shot. Add a Sokol adapter implementing `Nova::RHI::Device`, route one rendering subsystem through the adapter, verify the editor/game, and then migrate subsystem-by-subsystem.
