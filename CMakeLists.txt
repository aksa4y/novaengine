cmake_minimum_required(VERSION 3.20)
project(NovaEngine LANGUAGES C CXX)

set(NOVA_ENGINE_VERSION "0.1.0" CACHE STRING "Nova Engine version")

# Nova public architecture: Editor -> Runtime -> RHI.
add_subdirectory(rhi)
add_subdirectory(runtime)
add_subdirectory(editor)

# Optional Windows game executable used as the first native application shell.
option(NOVA_BUILD_WIN32_GAME "Build the Windows NovaGame application" OFF)
if(NOVA_BUILD_WIN32_GAME AND WIN32)
    add_subdirectory(samples/nova-win32)
endif()

# Nova public API smoke tests. These depend only on Nova Runtime/RHI.
option(NOVA_BUILD_TESTS "Build Nova public API smoke tests" ON)
if(NOVA_BUILD_TESTS)
    enable_testing()
    add_executable(nova-runtime-entry-smoke
        runtime/tests/NovaRuntimeEntrySmoke.cpp
    )
    target_link_libraries(nova-runtime-entry-smoke PRIVATE Nova::Runtime)
    target_compile_features(nova-runtime-entry-smoke PRIVATE cxx_std_17)
    add_test(NAME nova-runtime-entry-smoke COMMAND nova-runtime-entry-smoke)
endif()

# The complete upstream build graph is preserved verbatim under the explicit
# legacy boundary while the implementation is migrated into the Nova runtime tree.
set(NOVA_LEGACY_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/legacy/CMakeLists.txt")
if(EXISTS "${NOVA_LEGACY_CMAKE}")
    include("${NOVA_LEGACY_CMAKE}")
endif()

if(TARGET nova-editor-core AND NOT TARGET Nova::Editor)
    add_library(Nova::Editor ALIAS nova-editor-core)
endif()

message(STATUS "Nova Engine ${NOVA_ENGINE_VERSION}")
message(STATUS "Nova architecture: Nova::Editor -> Nova::Runtime -> Nova::RHI")
