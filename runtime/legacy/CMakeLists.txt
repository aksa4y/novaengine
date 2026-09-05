# Nova Runtime legacy implementation build boundary.
# This file intentionally preserves the original engine target definition while
# its implementation is being migrated into the new runtime/ tree.
#
# The original build script is kept verbatim in the compatibility mirror at
# engine/CMakeLists.txt during the staged migration. This wrapper is the future
# insertion point for the Nova-native runtime target.

if(NOT TARGET doriax)
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../engine/CMakeLists.txt")
endif()
