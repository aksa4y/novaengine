# Nova Engine migration status

## Current architecture

```text
Game / Editor
      |
      v
Nova::Runtime
      |
      v
Nova::RHI
      |
      +---- Null backend
      +---- Sokol compatibility seam
      +---- Vulkan / D3D / Metal / WebGPU (planned)
```

## What is active

- `rhi/` contains the public rendering abstraction and backend factories.
- `runtime/` contains the new public runtime boundary.
- `editor/` contains the new public editor boundary.
- Legacy `engine/` remains intact as the implementation source while the
  mechanical migration is in progress.
- `cmake/NovaRuntime.cmake` is the single migration integration point for
  connecting the legacy runtime to Nova targets.

## Compatibility rule

New code should depend on namespaced Nova targets:

- `Nova::RHI`
- `Nova::Runtime`
- `Nova::EditorCore`

New code must not add a direct dependency from Runtime to Editor.

The legacy `doriax` target may remain temporarily as an implementation detail
behind `Nova::Runtime` until the source tree is physically moved.

## Verification

The RHI can be configured as a standalone CMake project with its smoke test:

```text
cmake -S rhi -B build-rhi
cmake --build build-rhi
ctest --test-dir build-rhi --output-on-failure
```

The full-editor build still uses the legacy root CMake orchestration until the
large root file is migrated in the next mechanical step.
