#include "order_book.h"
#include <algorithm>
#include <numeric>

Trades OrderBook::AddOrder(OrderPointer order) {
    if (orders_.find(order->GetOrderId()) != orders_.end()) {
        return { };
    }
    if (order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice())) {
        return { };
    }

    OrderPointers::iterator iterator;

    if (order->GetSide() == Side::Buy) {
        auto& orders = bids_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size() - 1);
    }
    else {
        auto& orders = asks_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size() - 1);
    }

    orders_.insert({ order->GetOrderId(), OrderEntry{ order, iterator } });

    return MatchOrders();
}

void OrderBook::CancelOrder(OrderId orderId) {
    if (orders_.find(orderId) == orders_.end()) {
        return;
    }

    const auto& [order, iterator] = orders_.at(orderId);

    if (order->GetSide() == Side::Sell) {
        auto price = order->GetPrice();
        auto& orders = asks_.at(price);
        orders.erase(iterator);
        if (orders.empty()) {
            asks_.erase(price);
        }
    }
    else {
        auto price = order->GetPrice();
        auto& orders = bids_.at(price);
        orders.erase(iterator);
        if (orders.empty()) {
            bids_.erase(price);
        }
    }
    orders_.erase(orderId);
}

Trades OrderBook::MatchOrder(OrderModify order) {
    if (orders_.find(order.GetOrderId()) == orders_.end()) {
        return { };
    }
    const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
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
        [](std::size_t runningSum, const OrderPointer& order)
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

            trades.push_back(Trade{
                TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity },
                TradeInfo{ ask->GetOrderId(), ask->GetPrice(), quantity }
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

    // Handle Fill-and-Kill orders that couldn't be fully matched
    std::vector<OrderId> ordersToCancel;
    for (const auto& [orderId, orderEntry] : orders_) {
        if (orderEntry.order_->GetOrderType() == OrderType::FillAndKill) {
            ordersToCancel.push_back(orderId);
        }
    }
    
    for (OrderId orderId : ordersToCancel) {
        CancelOrder(orderId);
    }

    return trades;
}
