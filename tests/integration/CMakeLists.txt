# Integration Tests
add_executable(integration_tests thread_safety_test.cpp)

# Link with core library and threading
target_link_libraries(integration_tests orderbook_core)

# Set target properties
set_target_properties(integration_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Add to CTest
add_test(NAME IntegrationTests COMMAND integration_tests)
