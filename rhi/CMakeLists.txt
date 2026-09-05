cmake_minimum_required(VERSION 3.20)

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
