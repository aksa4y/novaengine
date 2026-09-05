# Nova compatibility boundary for the original engine build graph.
# Keep the upstream build orchestration isolated here while the implementation
# is migrated into the Nova runtime tree.
if(NOT TARGET doriax)
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../engine/CMakeLists.txt")
endif()

# Public Nova names hide the legacy implementation target from new consumers.
if(TARGET doriax AND NOT TARGET Nova::Engine)
    add_library(Nova::Engine ALIAS doriax)
endif()
