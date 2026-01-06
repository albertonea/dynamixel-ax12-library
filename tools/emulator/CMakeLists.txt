add_executable(emulator src/emulator.c)

target_include_directories(emulator PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(emulator PRIVATE dynamixel_motor)
