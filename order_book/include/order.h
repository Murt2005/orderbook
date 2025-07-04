#pragma once

#include "types.h"
#include <memory>
#include <list>

// Class representing an individual order in system
// Manages order properties including type, side, price, and quantity
// Tracks both initial and remaining quantities to support partial fills
// Provides methods for order filling and state querying
class Order {
public:
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity);

    OrderId GetOrderId() const;
    Side GetSide() const;
    Price GetPrice() const;
    OrderType GetOrderType() const;
    Quantity GetInitialQuantity() const;
    Quantity GetRemainingQuantity() const;
    Quantity GetFilledQuantity() const;
    bool IsFilled() const;

    // Fills the order by reducing its remaining quantity
    // Throws exception if attempting to fill more than remaining quantity
    void Fill(Quantity quantity);

private:
    OrderType orderType_;
    OrderId orderId_;
    Side side_;
    Price price_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;
