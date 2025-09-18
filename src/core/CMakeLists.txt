# Core Order Book Library
set(CORE_SOURCES
    src/order_book.cpp
    src/order.cpp
    src/trade.cpp
    src/order_modify.cpp
    src/order_book_level_infos.cpp
    src/performance_tracker.cpp
)

# Create the core library
add_library(orderbook_core STATIC ${CORE_SOURCES})

# Set target properties
set_target_properties(orderbook_core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(orderbook_core PUBLIC include)

# Link threading library
find_package(Threads REQUIRED)
target_link_libraries(orderbook_core Threads::Threads)
