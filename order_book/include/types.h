#pragma once

#include <cstdint>

// Defines different types of orders supported by the order book.
// - GoodTillCancel: Order remains active until explicitly cancelled
// - FillAndKill: Order is immediately filled or cancelled if it cannot be matched
enum class OrderType {
    GoodTillCancel,
    FillAndKill
};

// Defines the side of an order (buy or sell).
enum class Side {
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;
