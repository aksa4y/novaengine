# Nova Runtime facade.
#
# The original engine sources remain in engine/ during the migration so that
# existing editor/export paths continue to work. This target is the stable
# Runtime boundary that the Editor and future game projects will consume.

add_library(nova-runtime INTERFACE)
add_library(Nova::Runtime ALIAS nova-runtime)

target_link_libraries(nova-runtime INTERFACE Nova::RHI)

target_include_directories(nova-runtime INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine
)

# Migration note: the legacy engine target is attached by the root build until
# its sources are physically moved into runtime/ in the next migration phase.
if(TARGET doriax)
    target_link_libraries(nova-runtime INTERFACE doriax)
endif()
