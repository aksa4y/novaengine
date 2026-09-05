# Nova Engine migration status

The active Nova development line is `feature/nova-foundation`.

Public dependency direction:

`Nova::EditorCore -> Nova::Runtime -> Nova::RHI`

The root build entry point is `NovaEngine`. The original upstream CMake graph
is preserved under `cmake/legacy/CMakeLists.txt` while the implementation is
migrated into the Nova runtime tree.

The project-owned rebrand is being applied outside third-party vendor trees;
`libs/` is intentionally excluded from identifier replacement.

Full native build verification is not claimed from this environment.
