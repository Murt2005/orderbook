#!/bin/bash

# Build script for Order Book project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building Order Book project...${NC}"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${YELLOW}Binaries are available in:${NC}"
echo "  - Main application: build/bin/main"
echo "  - Unit tests: build/tests/unit_tests"
echo "  - Integration tests: build/tests/integration_tests"
echo "  - Benchmarks: build/tests/performance_benchmark"
