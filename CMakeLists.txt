cmake_minimum_required(VERSION 3.16)
project(robotarm VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

set(DYNAMIXEL_O "${CMAKE_SOURCE_DIR}/vendor/dynamixel/lib/dynamixel.o")
set(DYNAMIXEL_INCLUDE "${CMAKE_SOURCE_DIR}/vendor/dynamixel/include")

add_library(dynamixel_motor
        src/dynamixel_motor.c
        src/dynamixel_protocol.c
        src/dynamixel_translation.c
        src/dynamixel_translation.h
        src/dynamixel_constants.h
)
target_include_directories(dynamixel_motor PUBLIC include PRIVATE vendor/dynamixel/include)

add_subdirectory(tools/emulator)
add_subdirectory(tools/hanoi-solver)
