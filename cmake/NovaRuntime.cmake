# Nova Runtime integration layer.
#
# Intended dependency direction:
#   Nova::EditorCore -> Nova::Runtime -> Nova::RHI
#
# The legacy implementation is isolated from the public Nova target graph.

include_guard(GLOBAL)

if(NOT TARGET Nova::RHI)
    message(FATAL_ERROR "NovaRuntime.cmake requires rhi/ to be added before it")
endif()

if(NOT TARGET Nova::Runtime)
    message(FATAL_ERROR "NovaRuntime.cmake requires runtime/ to be added before it")
endif()

# Compatibility target for untouched legacy implementation code. New code must
# consume Nova::Runtime and never link the legacy target directly.
if(TARGET doriax AND NOT TARGET Nova::LegacyRuntime)
    add_library(nova-legacy-runtime INTERFACE)
    add_library(Nova::LegacyRuntime ALIAS nova-legacy-runtime)
    target_link_libraries(nova-legacy-runtime INTERFACE doriax)
endif()

set(NOVA_RUNTIME_READY TRUE CACHE INTERNAL "Nova Runtime integration is configured")
