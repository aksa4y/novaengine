# Nova Editor build boundary.
# Editor -> Runtime -> RHI. Runtime never depends on Editor.

add_library(nova-editor-core INTERFACE)
add_library(Nova::EditorCore ALIAS nova-editor-core)
add_library(Nova::Editor ALIAS nova-editor-core)

target_link_libraries(nova-editor-core INTERFACE Nova::Runtime)

target_include_directories(nova-editor-core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(nova-editor-core INTERFACE cxx_std_17)

# The legacy editor executable remains assembled by the compatibility build.
# New editor-facing code must consume Nova::EditorCore only.
