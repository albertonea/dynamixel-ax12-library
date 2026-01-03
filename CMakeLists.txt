cmake_minimum_required(VERSION 3.16)
project(robotarm VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

# Build the main library
add_library(robotarm
        src/dynamixel_motor.c
        src/dynamixel_protocol.c
)
target_include_directories(robotarm PUBLIC include PRIVATE vendor/dynamixel/include)
#target_link_libraries(robotarm PRIVATE "${CMAKE_SOURCE_DIR}/vendor/dynamixel/lib/dynamixel.o")

# Add tools subdirectories
add_subdirectory(tools/emulator)
add_subdirectory(tools/hanoi-solver)

# Optional: Install rules
#install(TARGETS robotarm DESTINATION lib)
#install(DIRECTORY include/ DESTINATION include)
