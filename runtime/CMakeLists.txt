# Nova Runtime boundary.
#
# The legacy engine target is kept as an implementation dependency during the
# migration. New game-facing APIs live under runtime/include and link through
# Nova::RHI instead of exposing editor implementation details.

add_library(nova-runtime STATIC
    src/Runtime.cpp
)
add_library(Nova::Runtime ALIAS nova-runtime)

target_link_libraries(nova-runtime
    PUBLIC
        Nova::RHI
)

if(TARGET doriax)
    target_link_libraries(nova-runtime PRIVATE doriax)
endif()

target_include_directories(nova-runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../engine
)

target_compile_features(nova-runtime PUBLIC cxx_std_17)
