# Unit Tests
add_executable(unit_tests test_main.cpp)

# Link with core library
target_link_libraries(unit_tests orderbook_core)

# Set target properties
set_target_properties(unit_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Add to CTest
add_test(NAME UnitTests COMMAND unit_tests)
