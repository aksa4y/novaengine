# Nova Engine migration status

The active Nova development line is `feature/nova-foundation`.

Public dependency direction:

`Nova::EditorCore -> Nova::Runtime -> Nova::RHI`

The root build entry point is named `NovaEngine`. The original upstream build
orchestration is preserved under `cmake/legacy/CMakeLists.txt` during the
mechanical migration so the source graph is not lost.

The repository rebrand is being performed outside third-party vendor trees.
`libs/` remains excluded from project-owned identifier replacement.
