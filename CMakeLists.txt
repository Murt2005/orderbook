cmake_minimum_required(VERSION 3.15)
project(OrderBook VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# Include directories
include_directories(src/core/include)

# Core library
add_subdirectory(src/core)

# Applications
add_subdirectory(src/applications/main)

# Tests
enable_testing()
add_subdirectory(tests/unit)
add_subdirectory(tests/integration)
add_subdirectory(tests/benchmarks)

# Dashboard (when implemented)
# add_subdirectory(src/dashboard)

# Installation
install(TARGETS orderbook_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY src/core/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
