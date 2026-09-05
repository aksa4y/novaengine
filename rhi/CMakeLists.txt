cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(NovaRHI LANGUAGES CXX)
    option(NOVA_RHI_BUILD_TESTS "Build the standalone Nova RHI smoke test" ON)
endif()

add_library(nova-rhi STATIC
    src/NullRHI.cpp
)

add_library(Nova::RHI ALIAS nova-rhi)

target_include_directories(nova-rhi
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(nova-rhi PUBLIC cxx_std_17)

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
