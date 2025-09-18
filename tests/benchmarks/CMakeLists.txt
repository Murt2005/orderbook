# Performance Benchmarks
add_executable(performance_benchmark performance_benchmark.cpp)

# Link with core library
target_link_libraries(performance_benchmark orderbook_core)

# Set target properties
set_target_properties(performance_benchmark PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Add to CTest
add_test(NAME PerformanceBenchmark COMMAND performance_benchmark)
