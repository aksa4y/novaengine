# Nova Engine architecture

```text
Nova Engine
├── runtime/              # shipping runtime boundary
│   ├── core/              # ECS, scene, objects, scripting, etc.
│   ├── platform/          # platform implementations
│   └── renders/           # renderer adapters
├── rhi/                   # API-neutral rendering interface
│   ├── include/nova/rhi/
│   └── src/
├── editor/                # editor-only tools and UI
├── shadercompiler/        # offline shader tooling
├── libs/                  # third-party dependencies
└── tools/                 # future cooker/packager/CLI tools
```

## Dependency direction

```text
Editor ───────► Runtime ───────► RHI
   │                              │
   └──── editor-only tools        └── Vulkan/D3D/Metal/OpenGL/WebGPU/Sokol adapters
```

The Runtime must never depend on Editor. The RHI must never depend on Editor.

## Migration policy

The existing `engine/` directory is kept temporarily to avoid breaking the project during the refactor. It is a compatibility source tree, not the final architecture. Physical source relocation and target renaming happen after the boundaries compile independently.
