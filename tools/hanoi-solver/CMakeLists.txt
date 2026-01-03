add_executable(hanoi-solver src/hanoi-solver.c)
target_include_directories(hanoi-solver
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/vendor/dynamixel/include
)
target_link_libraries(hanoi-solver PRIVATE robotarm)
