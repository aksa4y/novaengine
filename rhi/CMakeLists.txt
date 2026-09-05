cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(NovaRHI LANGUAGES CXX)
    option(NOVA_RHI_BUILD_TESTS "Build the standalone Nova RHI smoke test" ON)
endif()

add_library(nova-rhi STATIC
    src/NullRHI.cpp
    src/SokolRHI.cpp
    src/NativeBackendStubs.cpp
    src/Capabilities.cpp
)

add_library(Nova::RHI ALIAS nova-rhi)

target_include_directories(nova-rhi
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(nova-rhi PUBLIC cxx_std_17)

target_compile_definitions(nova-rhi
    PUBLIC
        NOVA_RHI_ABI_VERSION=1
)

# Native SDK adapters are opt-in and platform-scoped. The portable target always
# links NativeBackendStubs.cpp so a missing SDK produces an explicit nullptr
# from the corresponding factory instead of a compile/link failure.
option(NOVA_RHI_ENABLE_VULKAN "Enable the native Vulkan adapter" OFF)
option(NOVA_RHI_ENABLE_D3D11 "Enable the native D3D11 adapter" OFF)
option(NOVA_RHI_ENABLE_D3D12 "Enable the native D3D12 adapter" OFF)
option(NOVA_RHI_ENABLE_METAL "Enable the native Metal adapter" OFF)
option(NOVA_RHI_ENABLE_OPENGL "Enable the native OpenGL adapter" OFF)
option(NOVA_RHI_ENABLE_WEBGPU "Enable the native WebGPU adapter" OFF)

if(NOVA_RHI_ENABLE_VULKAN)
    message(STATUS "Nova RHI: Vulkan adapter requested")
endif()
if(NOVA_RHI_ENABLE_D3D11 OR NOVA_RHI_ENABLE_D3D12)
    message(STATUS "Nova RHI: Direct3D adapter requested")
endif()
if(NOVA_RHI_ENABLE_METAL)
    message(STATUS "Nova RHI: Metal adapter requested")
endif()
if(NOVA_RHI_ENABLE_OPENGL)
    message(STATUS "Nova RHI: OpenGL adapter requested")
endif()
if(NOVA_RHI_ENABLE_WEBGPU)
    message(STATUS "Nova RHI: WebGPU adapter requested")
endif()

set_target_properties(nova-rhi PROPERTIES
    OUTPUT_NAME "nova-rhi"
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOVA_RHI_BUILD_TESTS)
    enable_testing()
    add_executable(nova-rhi-smoke
        tests/RHISmoke.cpp
    )
    target_link_libraries(nova-rhi-smoke PRIVATE Nova::RHI)
    target_compile_features(nova-rhi-smoke PRIVATE cxx_std_17)
    add_test(NAME nova-rhi-smoke COMMAND nova-rhi-smoke)
endif()
