# Nova Runtime boundary.
#
# Runtime is deliberately independent from the Editor and from the legacy
# Doriax implementation target. Legacy sources may still be built by the root
# project, but the new game-facing Runtime API is a clean Nova layer over RHI.

add_library(nova-runtime STATIC
    src/Runtime.cpp
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
