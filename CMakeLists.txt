cmake_minimum_required(VERSION 3.20)
project(NovaEngine LANGUAGES C CXX)

# Nova Engine root build. The existing legacy orchestration remains included
# below during the staged Doriax -> Nova mechanical migration.
set(NOVA_ENGINE_VERSION "0.1.0" CACHE STRING "Nova Engine version")

# Public architecture targets.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/rhi/CMakeLists.txt")
    add_subdirectory(rhi)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/runtime/CMakeLists.txt")
    add_subdirectory(runtime)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/editor/CMakeLists.txt")
    add_subdirectory(editor)
endif()

# Preserve the original editor/runtime implementation while its large build
# graph is being migrated. This include is temporary and is the only place
# where the legacy Doriax target is allowed to enter the Nova build.
set(NOVA_LEGACY_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/engine/CMakeLists.txt")
if(EXISTS "${NOVA_LEGACY_CMAKE}")
    include("${NOVA_LEGACY_CMAKE}")
endif()

if(TARGET nova-editor-core AND TARGET Nova::Runtime)
    add_library(Nova::Editor ALIAS nova-editor-core)
endif()

message(STATUS "Nova Engine ${NOVA_ENGINE_VERSION}")
message(STATUS "Nova architecture targets: Nova::RHI, Nova::Runtime, Nova::EditorCore")
