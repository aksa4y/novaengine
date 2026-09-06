add_executable(nova-win32 WIN32
    main.cpp
)

target_link_libraries(nova-win32 PRIVATE Nova::Runtime)
target_compile_features(nova-win32 PRIVATE cxx_std_17)

target_link_libraries(nova-win32 PRIVATE user32 gdi32 shell32)

set_target_properties(nova-win32 PROPERTIES
    OUTPUT_NAME "NovaGame"
)
