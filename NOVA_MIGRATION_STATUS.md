# Nova Engine migration status

## Current architecture

```text
Game
 |
v
Nova::Runtime
 |
v
Nova::RHI
 |
+-- Null backend
+-- Sokol compatibility seam
+-- Vulkan / D3D / Metal / WebGPU (planned)

Editor
 |
v
Nova::EditorCore
 |
v
Nova::Runtime
```

## Active migration state

- `rhi/` contains the public rendering abstraction and backend factory.
- `runtime/` contains the new public Runtime boundary.
- `editor/` contains the new public Editor boundary.
- `Nova::Runtime` links only to `Nova::RHI`; it does not link to the legacy
  `doriax` target and does not include `engine/` headers.
- Legacy `engine/` remains intact as a temporary compatibility/build source.
- `runtime/legacy/CMakeLists.txt` is the compatibility seam for the old build.
- `cmake/NovaRuntime.cmake` remains the migration integration point used while
  the large legacy root build file is being mechanically replaced.

## Single-branch rule

All Nova migration work is developed on:

`feature/nova-foundation`

No new implementation commits should be placed on temporary migration
branches. Temporary branches created during earlier tooling experiments are
not part of the Nova development line.

## Dependency rule

New code uses only these public targets:

- `Nova::RHI`
- `Nova::Runtime`
- `Nova::EditorCore`

Required dependency direction:

`Editor -> Runtime -> RHI`

Runtime must never depend on Editor. RHI must never depend on Editor.

## Verification

Standalone RHI smoke test:

```text
cmake -S rhi -B build-rhi
cmake --build build-rhi
ctest --test-dir build-rhi --output-on-failure
```

A full native editor build is not claimed as verified in this environment;
the remaining mechanical step is replacing the legacy root CMake orchestration
without losing its platform/resource/shader configuration.
