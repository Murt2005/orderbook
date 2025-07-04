#pragma once

#include "types.h"
#include "order.h"
#include "order_modify.h"
#include "trade.h"
#include "order_book_level_infos.h"
#include <map>
#include <unordered_map>
#include <functional>


// Main order book class implementing a limit order book with price-time priority
// Manages bid and ask orders, handles order matching, and maintains order book state
// Features:
// - Price-time priority matching (best price first, then FIFO within price levels)
// - Support for Good-Till-Cancel (GTC) and Fill-and-Kill (FAK) orders
// - Order modification throuugh cancel-and-replace semantics
// - Real-time trade generation and order book level information
// - Efficient order lookup and management using hash maps and sorted containers
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

    // Determines if an incoming order can be matched against existing orders
    // Checks if there are oppposing orders at compatible price levels
    bool CanMatch(Side side, Price price) const;

    // Matching engine that executes trades between compatible orders
    // Implements price-time priority and handles partial fills
    // Returns a vector of all trades executed during the matching process
    Trades MatchOrders();
};
