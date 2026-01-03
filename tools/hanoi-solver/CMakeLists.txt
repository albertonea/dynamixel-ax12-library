add_executable(hanoi-solver src/hanoi-solver.c)
target_link_libraries(hanoi-solver PRIVATE robotarm)
target_include_directories(hanoi-solver PRIVATE ${CMAKE_SOURCE_DIR}/include)
