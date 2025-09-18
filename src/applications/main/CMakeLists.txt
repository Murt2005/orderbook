# Main Application
add_executable(main main.cpp)

# Link with core library
target_link_libraries(main orderbook_core)

# Set target properties
set_target_properties(main PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
