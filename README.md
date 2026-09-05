# Nova Engine

Nova Engine is the renamed and independently developed fork of the original
Doriax Engine codebase.

The project keeps the original engine capabilities while migrating its public
architecture to explicit Nova layers:

```text
Game / Application
        |
        v
Nova::Runtime
        |
        v
Nova::RHI

Nova::EditorCore
        |
        v
Nova::Runtime
```

## Project direction

Nova is being rebranded and mechanically migrated from the upstream Doriax
implementation. Third-party code under `libs/` remains third-party and is not
renamed.

The public targets are:

- `Nova::RHI` — graphics abstraction and backend selection
- `Nova::Runtime` — game/runtime-facing API
- `Nova::EditorCore` — editor-facing boundary

The dependency direction is strict: `Editor -> Runtime -> RHI`.

## Platforms

The inherited project targets desktop platforms and mobile/web export paths;
Nova's backend roadmap includes OpenGL, Vulkan, Direct3D, Metal and WebGPU,
with Android and HTML/WebAssembly treated as first-class runtime targets.

## Build

```text
cmake -S . -B build
cmake --build build --config Release
```

For the standalone RHI smoke test:

```text
cmake -S rhi -B build-rhi
cmake --build build-rhi
ctest --test-dir build-rhi --output-on-failure
```

The original upstream root build graph is preserved in
`cmake/legacy/CMakeLists.txt` while the final source migration is completed.

## License

Nova Engine source retains the upstream MIT license. Third-party dependencies
remain under their respective licenses.
