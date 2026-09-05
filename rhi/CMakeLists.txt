cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(NovaRHI LANGUAGES C CXX)
    option(NOVA_RHI_BUILD_TESTS "Build the standalone Nova RHI smoke test" ON)
endif()

option(NOVA_RHI_ENABLE_VULKAN "Enable the native Vulkan adapter" OFF)
option(NOVA_RHI_ENABLE_D3D11 "Enable the native D3D11 adapter" OFF)
option(NOVA_RHI_ENABLE_D3D12 "Enable the native D3D12 adapter" OFF)
option(NOVA_RHI_ENABLE_METAL "Enable the native Metal adapter" OFF)
option(NOVA_RHI_ENABLE_OPENGL "Enable the native OpenGL adapter" OFF)
option(NOVA_RHI_ENABLE_WEBGPU "Enable the native WebGPU adapter" OFF)

set(NOVA_RHI_SOURCES
    src/NullRHI.cpp
    src/SokolRHI.cpp
    src/NativeBackendStubs.cpp
    src/Capabilities.cpp
    src/BackendSelection.cpp
    src/VulkanRHI.cpp
    src/D3D11RHI.cpp
    src/D3D12RHI.cpp
)

if(NOVA_RHI_ENABLE_METAL)
    if(NOT APPLE)
        message(FATAL_ERROR "NOVA_RHI_ENABLE_METAL requires an Apple platform")
    endif()
    enable_language(OBJCXX)
    list(APPEND NOVA_RHI_SOURCES src/MetalRHI.mm)
endif()

add_library(nova-rhi STATIC ${NOVA_RHI_SOURCES})
add_library(Nova::RHI ALIAS nova-rhi)

target_include_directories(nova-rhi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(nova-rhi PUBLIC cxx_std_17)
target_compile_definitions(nova-rhi PUBLIC NOVA_RHI_ABI_VERSION=1)

if(NOVA_RHI_ENABLE_VULKAN)
    find_package(Vulkan REQUIRED)
    target_compile_definitions(nova-rhi PRIVATE NOVA_RHI_NATIVE_VULKAN=1)
    target_link_libraries(nova-rhi PRIVATE Vulkan::Vulkan)
endif()

if(NOVA_RHI_ENABLE_D3D11)
    if(NOT WIN32)
        message(FATAL_ERROR "NOVA_RHI_ENABLE_D3D11 requires Windows")
    endif()
    target_compile_definitions(nova-rhi PRIVATE NOVA_RHI_NATIVE_D3D11=1)
    target_link_libraries(nova-rhi PRIVATE d3d11 dxgi)
endif()

if(NOVA_RHI_ENABLE_D3D12)
    if(NOT WIN32)
        message(FATAL_ERROR "NOVA_RHI_ENABLE_D3D12 requires Windows")
    endif()
    target_compile_definitions(nova-rhi PRIVATE NOVA_RHI_NATIVE_D3D12=1)
    target_link_libraries(nova-rhi PRIVATE d3d12 dxgi)
endif()

if(NOVA_RHI_ENABLE_METAL)
    target_compile_definitions(nova-rhi PRIVATE NOVA_RHI_NATIVE_METAL=1)
    target_link_libraries(nova-rhi PRIVATE "-framework Metal")
endif()

if(NOVA_RHI_ENABLE_OPENGL)
    target_compile_definitions(nova-rhi PRIVATE NOVA_RHI_NATIVE_OPENGL=1)
endif()

if(NOVA_RHI_ENABLE_WEBGPU)
    target_compile_definitions(nova-rhi PRIVATE NOVA_RHI_NATIVE_WEBGPU=1)
endif()

set_target_properties(nova-rhi PROPERTIES
    OUTPUT_NAME "nova-rhi"
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOVA_RHI_BUILD_TESTS)
    enable_testing()
    add_executable(nova-rhi-smoke tests/RHISmoke.cpp)
    target_link_libraries(nova-rhi-smoke PRIVATE Nova::RHI)
    target_compile_features(nova-rhi-smoke PRIVATE cxx_std_17)
    add_test(NAME nova-rhi-smoke COMMAND nova-rhi-smoke)
endif()
