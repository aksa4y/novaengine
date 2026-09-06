# Nova Runtime boundary.
#
# Runtime is deliberately independent from the Editor and from the legacy
# Doriax implementation target. Legacy sources may still be built by the root
# project, but the new game-facing Runtime API is a clean Nova layer over RHI.

add_library(nova-runtime STATIC
    src/Runtime.cpp
    src/Renderer.cpp
    src/Mesh.cpp
)
add_library(Nova::Runtime ALIAS nova-runtime)

target_link_libraries(nova-runtime
    PUBLIC
        Nova::RHI
)

target_include_directories(nova-runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(nova-runtime PUBLIC cxx_std_17)

if(BUILD_TESTING)
    add_executable(nova-runtime-renderer-smoke tests/RendererSmoke.cpp)
    target_link_libraries(nova-runtime-renderer-smoke PRIVATE Nova::Runtime)
    target_compile_features(nova-runtime-renderer-smoke PRIVATE cxx_std_17)
    add_test(NAME nova-runtime-renderer-smoke COMMAND nova-runtime-renderer-smoke)

    add_executable(nova-runtime-mesh-smoke tests/MeshSmoke.cpp)
    target_link_libraries(nova-runtime-mesh-smoke PRIVATE Nova::Runtime)
    target_compile_features(nova-runtime-mesh-smoke PRIVATE cxx_std_17)
    add_test(NAME nova-runtime-mesh-smoke COMMAND nova-runtime-mesh-smoke)
endif()
