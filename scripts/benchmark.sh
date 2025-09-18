#!/bin/bash

# Benchmark script for Order Book project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Running Order Book benchmarks...${NC}"

# Build if not already built
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Building project first...${NC}"
    ./scripts/build.sh
fi

cd build

# Run benchmarks
echo -e "${YELLOW}Running performance benchmarks...${NC}"
./tests/performance_benchmark

echo -e "${GREEN}Benchmarks completed!${NC}"
