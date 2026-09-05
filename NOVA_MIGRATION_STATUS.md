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

- `rhi/` contains the public Nova rendering abstraction and backend factory.
- `runtime/` contains the public Nova Runtime boundary.
- `editor/` contains the public Nova Editor boundary.
- The original upstream CMake graph is preserved at `cmake/legacy/CMakeLists.txt`.
- The root build entry point is now `NovaEngine` and exposes the public dependency direction `Editor -> Runtime -> RHI`.
- Legacy `engine/` remains temporarily while its implementation is migrated into `runtime/`.
- New code must use `Nova::RHI`, `Nova::Runtime`, and `Nova::EditorCore`.

## Rebrand

The repository is being mechanically rebranded from Doriax to Nova. The
one-time rebrand script lives at `tools/nova/rebrand.py` and deliberately skips
vendored third-party code under `libs/`.

## Single branch

All active Nova development is performed on `feature/nova-foundation`.

## Verification

The standalone RHI smoke test remains:

```text
cmake -S rhi -B build-rhi
cmake --build build-rhi
ctest --test-dir build-rhi --output-on-failure
```

The full native editor build has not been claimed as verified from this
environment.
