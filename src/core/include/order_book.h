#pragma once

#include "types.h"
#include "order.h"
#include "order_modify.h"
#include "trade.h"
#include "order_book_level_infos.h"
#include "performance_tracker.h"
#include <map>
#include <unordered_map>
#include <functional>
#include <shared_mutex>
#include <mutex>
#include <atomic>


// Thread-safe order book class implementing a limit order book with price-time priority
// Manages bid and ask orders, handles order matching, and maintains order book state
// Features:
// - Thread-safe operations with read-write locks for optimal concurrency
// - Price-time priority matching (best price first, then FIFO within price levels)
// - Support for Good-Till-Cancel (GTC), ImmediateOrCancel (IOC), and FillOrKill (FOK) orders
// - Order modification through cancel-and-replace semantics
// - Real-time trade generation and order book level information
// - Efficient order lookup and management using hash maps and sorted containers
// - Atomic operations for processing state management
class OrderBook {
public:
    // Adds a new order to the order book and triggers matching
    // Returns any trades that result from the order addition
    Trades AddOrder(OrderPointer order);

    // Removes an order from the order book without triggering matching
    // Handles cleanup of empty price levels
    void CancelOrder(OrderId orderId);

    // Modifies an existing order using cancel-and-replace
    // Cancels the original order and adds a new one with updated parameters
    Trades MatchOrder(OrderModify order);

    // Returns the total number of active orders in the book
    std::size_t Size() const;

    // Gives a snapshot of current order book levels
    // Aggregates orders by price level and returns bid/ask information
    OrderBookLevelInfos GetOrderInfos() const;

    // Performance tracking methods
    void enablePerformanceTracking(bool enabled) { performanceTracker_.setEnabled(enabled); }
    bool isPerformanceTrackingEnabled() const { return performanceTracker_.isEnabled(); }
    void resetPerformanceMetrics() { performanceTracker_.reset(); }
    void printPerformanceReport() const { performanceTracker_.printReport(); }
    void printPerformanceSummary() const { performanceTracker_.printSummary(); }
    
    // Utility methods
    void clear() { 
        std::unique_lock<std::shared_mutex> lock(orderBookMutex_);
        bids_.clear();
        asks_.clear();
        orders_.clear();
    }

private:
    // Links and order to its position in the price level queue
    // Used for efficient order cancellation and modification
    struct OrderEntry {
        OrderPointer order_{ nullptr };           // Pointer to the order
        OrderPointers::iterator location_;        // Iterator to order's position in price level
    };

    // Price levels organized for optimal matching
    std::map<Price, OrderPointers, std::greater<Price>> bids_;  // Bids sorted high to low
    std::map<Price, OrderPointers, std::less<Price>> asks_;     // Asks sorted low to high
    std::unordered_map<OrderId, OrderEntry> orders_;           // Fast order lookup

    // Thread synchronization primitives
    mutable std::shared_mutex orderBookMutex_;                 // Protects main order book data structures
    mutable std::mutex performanceMutex_;                      // Protects performance tracking
    std::atomic<bool> isProcessing_{false};                    // Atomic flag for processing state

    // Determines if an incoming order can be matched against existing orders
    // Checks if there are oppposing orders at compatible price levels
    bool CanMatch(Side side, Price price) const;

    // Determines if an order can be completely filled or not
    bool CanFillCompletely(Side side, Price price, Quantity quantity) const;

    // Matching engine that executes trades between compatible orders
    // Implements price-time priority and handles partial fills
    // Returns a vector of all trades executed during the matching process
    Trades MatchOrders();

    // Thread-safe helper methods
    Trades AddOrderInternal(OrderPointer order);
    void CancelOrderInternal(OrderId orderId);
    Trades MatchOrderInternal(OrderModify order);
    std::size_t SizeInternal() const;
    OrderBookLevelInfos GetOrderInfosInternal() const;

    // Performance tracker instance
    mutable PerformanceTracker performanceTracker_;
};
