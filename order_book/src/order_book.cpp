#include "order_book.h"
#include <algorithm>
#include <numeric>

Trades OrderBook::AddOrder(OrderPointer order) {
    // Input validation
    if (!order) {
        return { };
    }
    if (order->GetRemainingQuantity() == 0) {
        return { }; // Reject zero quantity orders
    }
    if (order->GetOrderId() == 0) {
        return { }; // Reject invalid order ID
    }
    
    if (orders_.find(order->GetOrderId()) != orders_.end()) {
        return { };
    }
    if (order->GetOrderType() == OrderType::ImmediateOrCancel && !CanMatch(order->GetSide(), order->GetPrice())) {
        return { };
    }
    if (order->GetOrderType() == OrderType::FillOrKill && !CanFillCompletely(order->GetSide(), order->GetPrice(), order->GetRemainingQuantity())) {
        return { };
    }

    OrderPointers::iterator iterator;

    if (order->GetSide() == Side::Buy) {
        auto& orders = bids_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }
    else {
        auto& orders = asks_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }

    orders_.insert({ order->GetOrderId(), OrderEntry{ order, iterator } });

    return MatchOrders();
}

void OrderBook::CancelOrder(OrderId orderId) {
    auto orderIt = orders_.find(orderId);
    if (orderIt == orders_.end()) {
        return;
    }

    const auto& [order, iterator] = orderIt->second;

    if (order->GetSide() == Side::Sell) {
        auto price = order->GetPrice();
        auto priceIt = asks_.find(price);
        if (priceIt != asks_.end()) {
            auto& orders = priceIt->second;
            orders.erase(iterator);
            if (orders.empty()) {
                asks_.erase(price);
            }
        }
    }
    else {
        auto price = order->GetPrice();
        auto priceIt = bids_.find(price);
        if (priceIt != bids_.end()) {
            auto& orders = priceIt->second;
            orders.erase(iterator);
            if (orders.empty()) {
                bids_.erase(price);
            }
        }
    }
    orders_.erase(orderId);
}

Trades OrderBook::MatchOrder(OrderModify order) {
    auto orderIt = orders_.find(order.GetOrderId());
    if (orderIt == orders_.end()) {
        return { };
    }
    const auto& [existingOrder, _] = orderIt->second;
    OrderType orderType = existingOrder->GetOrderType();
    CancelOrder(order.GetOrderId());
    return AddOrder(order.ToOrderPointer(orderType));
}

std::size_t OrderBook::Size() const { return orders_.size(); }

OrderBookLevelInfos OrderBook::GetOrderInfos() const {
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(orders_.size());
    askInfos.reserve(orders_.size());

    auto CreateLevelInfos = [](Price price, const OrderPointers& orders) {
        return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), (Quantity)0,
        [](Quantity runningSum, const OrderPointer& order)
        { return runningSum + order->GetRemainingQuantity(); }) };
        };

    for (const auto& [price, orders] : bids_) {
        bidInfos.push_back(CreateLevelInfos(price, orders));
    }

    for (const auto& [price, orders] : asks_) {
        askInfos.push_back(CreateLevelInfos(price, orders));
    }

    return OrderBookLevelInfos{ bidInfos, askInfos };
}

bool OrderBook::CanMatch(Side side, Price price) const {
    if (side == Side::Buy) {
        if (asks_.empty()) {
            return false;
        }
        const auto& [bestAsk, _] = *asks_.begin();
        return price >= bestAsk;
    }
    else {
        if (bids_.empty()) {
            return false;
        }
        const auto& [bestBid, _] = *bids_.begin();
        return price <= bestBid;
    }
}

bool OrderBook::CanFillCompletely(Side side, Price price, Quantity quantity) const {
    Quantity availableQuantity = 0;

    if (side == Side::Buy) {
        for (const auto& [askPrice, orders] : asks_) {
            if (askPrice > price) {
                break;
            }
            for (const auto& order : orders) {
                availableQuantity += order->GetRemainingQuantity();
                if (availableQuantity >= quantity) {
                    return true;
                }
            }
        }
    } else {
        for (const auto& [bidPrice, orders] : bids_) {
            if (bidPrice < price) {
                break;
            }
            for (const auto& order : orders) {
                availableQuantity += order->GetRemainingQuantity();
                if (availableQuantity >= quantity) {
                    return true;
                }
            }
        }
    }
    return false;
}

Trades OrderBook::MatchOrders() {
    Trades trades;
    trades.reserve(orders_.size());

    while (true) {
        if (bids_.empty() || asks_.empty()) {
            break;
        }
        
        auto bidsIt = bids_.begin();
        auto asksIt = asks_.begin();
        
        Price bidPrice = bidsIt->first;
        Price askPrice = asksIt->first;
        
        if (bidPrice < askPrice) {
            break;
        }

        auto& bids = bidsIt->second;
        auto& asks = asksIt->second;

        while (!bids.empty() && !asks.empty()) {
            auto& bid = bids.front();
            auto& ask = asks.front();

            Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

            bid->Fill(quantity);
            ask->Fill(quantity);

            // Trade should execute at the price of the resting order (the one already in the book)
            // This gives price improvement to the aggressive order
            // The resting order's price is the better price for the aggressive order
            Price executionPrice = ask->GetPrice(); // Resting sell order's price
            
            trades.push_back(Trade{
                TradeInfo{ bid->GetOrderId(), executionPrice, quantity },
                TradeInfo{ ask->GetOrderId(), executionPrice, quantity }
            });

            if (bid->IsFilled()) {
                orders_.erase(bid->GetOrderId());
                bids.pop_front();
            }

            if (ask->IsFilled()) {
                orders_.erase(ask->GetOrderId());
                asks.pop_front();
            }
        }

        if (bids.empty()) {
            bids_.erase(bidPrice);
        }
        if (asks.empty()) {
            asks_.erase(askPrice);
        }
    }

    // Handle Immediate-Or-Cancel and FillOrKill orders that couldn't be fully matched
    std::vector<OrderId> ordersToCancel;
    ordersToCancel.reserve(orders_.size()); // Pre-allocate to avoid reallocations
    
    // Collect order IDs to cancel (avoiding iterator invalidation)
    for (const auto& [orderId, orderEntry] : orders_) {
        const auto& order = orderEntry.order_;
        if (order->GetOrderType() == OrderType::ImmediateOrCancel || 
            order->GetOrderType() == OrderType::FillOrKill) {
            ordersToCancel.push_back(orderId);
        }
    }
    
    // Cancel orders after collecting all IDs to avoid iterator invalidation
    for (OrderId orderId : ordersToCancel) {
        CancelOrder(orderId);
    }

    return trades;
}
