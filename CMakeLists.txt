cmake_minimum_required(VERSION 3.20)
project(NovaEngine LANGUAGES C CXX)

set(NOVA_ENGINE_VERSION "0.1.0" CACHE STRING "Nova Engine version")

# Nova public architecture: Editor -> Runtime -> RHI.
add_subdirectory(rhi)
add_subdirectory(runtime)
add_subdirectory(editor)

# The complete upstream build graph is preserved verbatim under the explicit
# legacy boundary while its implementation is being migrated into runtime/.
set(NOVA_LEGACY_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/legacy/CMakeLists.txt")
if(EXISTS "${NOVA_LEGACY_CMAKE}")
    include("${NOVA_LEGACY_CMAKE}")
endif()

if(TARGET nova-editor-core AND NOT TARGET Nova::Editor)
    add_library(Nova::Editor ALIAS nova-editor-core)
endif()

message(STATUS "Nova Engine ${NOVA_ENGINE_VERSION}")
message(STATUS "Nova architecture: Nova::Editor -> Nova::Runtime -> Nova::RHI")
