# Nova Editor is intentionally a separate build target.
# It may consume Nova::Runtime, but Runtime never depends on Editor.

add_library(nova-editor-core INTERFACE)
add_library(Nova::EditorCore ALIAS nova-editor-core)

target_link_libraries(nova-editor-core INTERFACE Nova::Runtime)

target_include_directories(nova-editor-core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# The existing editor executable is still assembled by the root CMake file in
# this migration stage. Moving the source list here is the next mechanical step.
