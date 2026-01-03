add_library(dynamixel OBJECT IMPORTED)

set_target_properties(dynamixel PROPERTIES
        IMPORTED_OBJECTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dynamixel.o"  # Full path to your .o
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
