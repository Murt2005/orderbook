#pragma once

#include "types.h"
#include "order.h"

// Class representing a request to modify and existing order
// Encapsulates the new order paramterers and provides conversions to Order object
// Used for order modification operations that cancel the original order
// and create a new one with updated parameters
class OrderModify {
public:
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity);

    OrderId GetOrderId() const;
    Price GetPrice() const;
    Side GetSide() const;
    Quantity GetQuantity() const;

    // Converts the modification request to a new Order object
    OrderPointer ToOrderPointer(OrderType type) const;

private:
    OrderId orderId_;
    Price price_;
    Side side_;
    Quantity quantity_;
};
