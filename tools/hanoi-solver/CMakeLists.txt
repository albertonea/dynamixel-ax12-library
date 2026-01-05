add_executable(hanoi-solver src/hanoi-solver.c)
target_include_directories(hanoi-solver
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/vendor/dynamixel/include
)

target_link_libraries(
        hanoi-solver
        PRIVATE dynamixel_motor ${DYNAMIXEL_O}
)

# Copy txt file to executable directory post-build
add_custom_command(TARGET hanoi-solver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/data/limits.txt
        $<TARGET_FILE_DIR:hanoi-solver>/limits.txt
        COMMENT "Copying limits.txt to executable directory"
)
