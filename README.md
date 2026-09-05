# Nova Engine

Nova Engine is the renamed and independently developed fork of the original
Doriax Engine codebase.

Public architecture:

```text
Application
   |
Nova::EditorCore -> Nova::Runtime -> Nova::RHI
```

`Nova::RHI` is the graphics abstraction, `Nova::Runtime` is the game-facing
runtime boundary, and `Nova::EditorCore` is the editor boundary. New code must
follow `Editor -> Runtime -> RHI` and must not introduce Runtime -> Editor or
RHI -> Editor dependencies.

The repository is being mechanically rebranded from Doriax to Nova. Vendored
third-party libraries under `libs/` are intentionally excluded from the rename.

The original upstream build graph is preserved at
`cmake/legacy/CMakeLists.txt` while the implementation is migrated into the
Nova runtime tree.

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

Standalone RHI smoke test:

```bash
cmake -S rhi -B build-rhi
cmake --build build-rhi
ctest --test-dir build-rhi --output-on-failure
```

## License

Nova Engine source retains the upstream MIT license. Third-party dependencies
remain under their respective licenses.
