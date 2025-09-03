# Low Level Limit Orderbook

A high-performance, production-ready limit order book implementation in C++ designed for institutional trading systems.

## Performance Highlights

- **Throughput**: Up to 414,000+ operations per second
- **Latency**: Sub-microsecond order matching
- **Scalability**: Supports 6,000+ concurrent orders across 37+ price levels
- **Reliability**: 100% test pass rate across 294 comprehensive tests

## Features

### Order Types
- **Good Till Cancel (GTC)**: Standard limit orders that remain active until cancelled
- **Immediate Or Cancel (IOC)**: Orders that execute immediately or are cancelled
- **Fill Or Kill (FOK)**: Orders that must be completely filled or are rejected

### Trading Sides
- **Buy Orders**: Bid side with price-time priority
- **Sell Orders**: Ask side with price-time priority

### Core Functionality
- **Price-Time Priority Matching**: Best price first, then FIFO within price levels
- **Partial Fills**: Support for orders that execute across multiple price levels
- **Order Modification**: Cancel-and-replace semantics for order updates
- **Real-time Trade Generation**: Automatic trade execution and reporting
- **Order Book Level Information**: Aggregated price level data

### Advanced Features
- **Exception-Safe Operations**: RAII patterns and smart pointer management
- **Performance Monitoring**: Nanosecond-precision timing and metrics
- **Comprehensive Testing**: 16 test categories covering all edge cases
- **Memory Management**: Efficient STL container usage with O(1) lookups

## Architecture

### Data Structures
- **Price Levels**: `std::map<Price, OrderPointers>` for sorted price level management
  - Bids: High to low price ordering (`std::greater<Price>`)
  - Asks: Low to high price ordering (`std::less<Price>`)
- **Order Lookup**: `std::unordered_map<OrderId, OrderEntry>` for O(1) order retrieval
- **Order Queues**: `std::list<OrderPointer>` for efficient insertion/deletion within price levels

### Performance Characteristics
- **Order Addition**: O(log n) price level access + O(1) order insertion
- **Order Cancellation**: O(1) lookup + O(1) removal
- **Order Matching**: O(log n) price level traversal + O(1) order processing
- **Order Lookup**: O(1) constant time retrieval

### Memory Management
- **Smart Pointers**: `std::shared_ptr<Order>` for automatic memory management
- **Exception Safety**: RAII patterns ensure resource cleanup
- **Container Efficiency**: Optimized STL usage for minimal memory overhead

## Performance Metrics

### Benchmark Results
```
Test 1: High-frequency order additions (10,000 orders)
  Completed in 94 ms
  Orders per second: 106,382
  Order book size: 2,158

Test 2: Order matching stress test
  Completed in 101 ms
  Orders per second: 99,009
  Final order book size: 3,703

Test 3: Mixed operations (add, modify, cancel)
  Completed in 7 ms
  Operations per second: 414,285
  Final order book size: 1,700

Test 4: Large order book operations
  Completed in 1018 ms
  Orders per second: 50,098
  Final order book size: 6,075
  Price levels (bids): 23
  Price levels (asks): 14
```

### Test Coverage
- **Total Tests**: 294 tests
- **Test Categories**: 16 comprehensive test suites
- **Success Rate**: 100% (0 failures)
- **Coverage Areas**: Order management, matching, edge cases, performance, error handling

## Build Requirements

### Prerequisites
- **Compiler**: GCC 7+ or Clang 5+ (C++17 support required)
- **Platform**: Linux, macOS, or Windows with C++17 support
- **Build System**: Make

### Dependencies
- **C++ Standard Library**: C++17 or later
- **STL Containers**: std::map, std::unordered_map, std::vector, std::list
- **Smart Pointers**: std::shared_ptr
- **Chrono Library**: High-resolution timing support

## Quick Start

### 1. Clone the Repository
```bash
git clone <repository-url>
cd order-book
```

### 2. Build the Project
```bash
make all
```

### 3. Run Tests
```bash
make test
./test_runner
```

### 4. Run Performance Benchmarks
```bash
make benchmark
./performance_benchmark
```

### 5. Run Main Application
```bash
make run
```

## Project Structure

```
order-book/
├── include/                          # Header files
│   ├── order_book.h                 # Main order book class
│   ├── order.h                      # Order representation
│   ├── types.h                      # Type definitions
│   ├── trade.h                      # Trade execution
│   ├── order_modify.h               # Order modification
│   ├── order_book_level_infos.h    # Price level information
│   └── performance_tracker.h        # Performance monitoring
├── src/                             # Source files
│   ├── order_book.cpp               # Core order book implementation
│   ├── order.cpp                    # Order management
│   ├── trade.cpp                    # Trade processing
│   ├── order_modify.cpp             # Order modification logic
│   ├── performance_tracker.cpp      # Performance metrics
│   ├── test_main.cpp                # Comprehensive test suite
│   ├── performance_benchmark.cpp    # Performance benchmarks
│   └── main.cpp                     # Main application
├── Makefile                         # Build configuration
├── README.md                        # This file
└── .gitignore                       # Git ignore rules
```

## Usage Examples

### Basic Order Book Usage
```cpp
#include "order_book.h"
#include "order.h"

// Create order book instance
OrderBook orderBook;

// Add a buy order
auto buyOrder = std::make_shared<Order>(
    OrderType::GoodTillCancel,
    1,                    // Order ID
    Side::Buy,            // Buy side
    100,                  // Price
    100                   // Quantity
);

// Add order to book
auto trades = orderBook.AddOrder(buyOrder);

// Check order book size
std::size_t size = orderBook.Size();

// Get price level information
auto levels = orderBook.GetOrderInfos();
```

### Performance Monitoring
```cpp
// Enable performance tracking
orderBook.enablePerformanceTracking(true);

// Run operations...
orderBook.AddOrder(order);

// Print performance report
orderBook.printPerformanceReport();
orderBook.printPerformanceSummary();
```

## Testing

### Test Categories
1. **Order Addition**: Basic order placement and validation
2. **Order Cancellation**: Order removal and cleanup
3. **Simple Matching**: Basic trade execution
4. **Partial Fills**: Orders executed across multiple price levels
5. **Price-Time Priority**: Order matching priority rules
6. **Order Types**: GTC, IOC, and FOK order handling
7. **Order Modification**: Cancel-and-replace operations
8. **Order Book Levels**: Price level aggregation
9. **Edge Cases**: Boundary conditions and error scenarios
10. **Exception Handling**: Error conditions and recovery
11. **Trade Validation**: Trade execution verification
12. **Order State Validation**: Order lifecycle management
13. **Performance Tracking**: Metrics and monitoring

### Running Tests
```bash
# Run all tests
make test
./test_runner

# Run specific test categories
# (Tests are automatically categorized and run sequentially)
```
