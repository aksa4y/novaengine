# Nova Runtime integration layer.
#
# This module is intentionally small enough to be included from the legacy root
# CMake without duplicating its platform/dependency logic.
#
# Intended dependency direction:
#   Nova::EditorCore -> Nova::Runtime -> Nova::RHI
#
# During migration the existing doriax target remains the implementation of the
# runtime. Once engine/ is moved into runtime/, NOVA_RUNTIME_LEGACY_TARGET can be
# removed and Nova::Runtime becomes the only runtime target.

include_guard(GLOBAL)

if(NOT TARGET Nova::RHI)
    message(FATAL_ERROR "NovaRuntime.cmake requires rhi/ to be added before it")
endif()

if(NOT TARGET Nova::Runtime)
    message(FATAL_ERROR "NovaRuntime.cmake requires runtime/ to be added before it")
endif()

# Make legacy engine consumption explicit rather than allowing editor code to
# link the old target directly. This is the migration boundary.
if(TARGET doriax AND NOT TARGET Nova::LegacyRuntime)
    add_library(nova-legacy-runtime INTERFACE)
    add_library(Nova::LegacyRuntime ALIAS nova-legacy-runtime)
    target_link_libraries(nova-legacy-runtime INTERFACE doriax)
endif()

# Optional compatibility switch for the current migration stage.
option(NOVA_RUNTIME_USE_LEGACY
    "Temporarily connect Nova::Runtime to the legacy doriax implementation"
    ON
)

if(NOVA_RUNTIME_USE_LEGACY AND TARGET nova-runtime AND TARGET doriax)
    get_target_property(_nova_runtime_links nova-runtime LINK_LIBRARIES)
    if(NOT _nova_runtime_links)
        set(_nova_runtime_links "")
    endif()

    list(FIND _nova_runtime_links doriax _nova_has_legacy)
    if(_nova_has_legacy EQUAL -1)
        target_link_libraries(nova-runtime PRIVATE doriax)
    endif()
endif()

# Consumers should use the namespaced Nova targets. The legacy target remains
# available solely for untouched upstream code during the mechanical migration.
set(NOVA_RUNTIME_READY TRUE CACHE INTERNAL "Nova Runtime integration is configured")
