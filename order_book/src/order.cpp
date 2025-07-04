#include "order.h"
#include <stdexcept>
#include <sstream>


Order::Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
        : orderType_{ orderType }
        , orderId_{ orderId }
        , side_{ side }
        , price_{ price }
        , initialQuantity_{ quantity }
        , remainingQuantity_{ quantity }
    { }

OrderId Order::GetOrderId() const {
    return orderId_;
}

Side Order::GetSide() const {
    return side_;
}

Price Order::GetPrice() const {
    return price_;
}

OrderType Order::GetOrderType() const {
    return orderType_;
}

Quantity Order::GetInitialQuantity() const {
    return initialQuantity_;
}

Quantity Order::GetRemainingQuantity() const {
    return remainingQuantity_;
}

Quantity Order::GetFilledQuantity() const {
    return GetInitialQuantity() - GetRemainingQuantity();
}

bool Order::IsFilled() const {
    return GetRemainingQuantity() == 0;
}

void Order::Fill(Quantity quantity) {
    if (quantity > GetRemainingQuantity()) {
        std::stringstream ss;
        ss << "Order (" << GetOrderId() << ") cannot be filled for more than its remaining quantity.";
        throw std::logic_error(ss.str());
    }
    remainingQuantity_ -= quantity;
}
