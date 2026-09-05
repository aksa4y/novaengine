# Nova Editor is intentionally a separate build target.
# Editor -> Runtime -> RHI. Runtime never depends on Editor.

add_library(nova-editor-core INTERFACE)
add_library(Nova::EditorCore ALIAS nova-editor-core)

target_link_libraries(nova-editor-core INTERFACE Nova::Runtime)

target_include_directories(nova-editor-core INTERFACE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# The existing executable is still assembled by the legacy root build during
# this migration. Its implementation will be switched to Nova::EditorCore in
# the next mechanical CMake migration once the legacy source staging is moved.
