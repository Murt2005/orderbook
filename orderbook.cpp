#include <iostream>
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <limits>
#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>
#include <sstream>

//
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

//Contains the price and total quantity available at that price level.
struct LevelInfo {
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

// Provides an overview of the order book's current state
// Encapsulates the bid and ask levels with their respective prices and quantities.
class OrderBookLevelInfos {
public:
    OrderBookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
        : bids_{ bids }
        , asks_{ asks }
    { }

    const LevelInfos& GetBids() const { return bids_; }
    const LevelInfos& GetAsks() const { return asks_; }

private:
    LevelInfos bids_;
    LevelInfos asks_;
};

// Class representing an individual order in system
// Manages order properties including type, side, price, and quantity
// Tracks both initial and remaining quantities to support partial fills
// Provides methods for order filling and state querying
class Order {
public:
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
        : orderType_{ orderType }
        , orderId_{ orderId }
        , side_{ side }
        , price_{ price }
        , initialQuantity_{ quantity }
        , remainingQuantity_{ quantity }
    { }

    OrderId GetOrderId() const { return orderId_; }
    Side GetSide() const { return side_; }
    Price GetPrice() const { return price_; }
    OrderType GetOrderType() const { return orderType_; }
    Quantity GetInitialQuantity() const { return initialQuantity_; }
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }
    Quantity GetFilledQuantity() const { return GetInitialQuantity() - GetRemainingQuantity(); }
    bool IsFilled() const { return GetRemainingQuantity() == 0; }

    // Fills the order by reducing its remaining quantity
    // Throws exception if attempting to fill more than remaining quantity
    void Fill(Quantity quantity) {
        if (quantity > GetRemainingQuantity()) {
            std::stringstream ss;
            ss << "Order (" << GetOrderId() << ") cannot be filled for more than its remaining quantity.";
            throw std::logic_error(ss.str());
        }
        remainingQuantity_ -= quantity;
    }

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

// Class representing a request to modify and existing order
// Encapsulates the new order paramterers and provides conversions to Order object
// Used for order modification operations that cancel the original order
// and create a new one with updated parameters
class OrderModify {
public:
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
        : orderId_{ orderId }
        , price_{ price }
        , side_{ side }
        , quantity_{ quantity }
    { }

    OrderId GetOrderId() const { return orderId_; }
    Price GetPrice() const { return price_; }
    Side GetSide() const { return side_; }
    Quantity GetQuantity() const { return quantity_; }

    // Converts the modification request to a new Order object
    OrderPointer ToOrderPointer(OrderType type) const {
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
    }

private:
    OrderId orderId_;
    Price price_;
    Side side_;
    Quantity quantity_;
};

// Contains information about one side of a trade execution
// Records the order ID, execution price, and filled quantity
struct TradeInfo {
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
};

// Class representing a completed trade between two orders
// Contains trade information for both the bid and ask sides
class Trade {
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        : bidTrade_{ bidTrade }
        , askTrade_{ askTrade }
    { }

    const TradeInfo& GetBidTrade() const { return bidTrade_; }
    const TradeInfo& GetAskTrade() const { return askTrade_; }

private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};

using Trades = std::vector<Trade>;

// Main order book class implementing a limit order book with price-time priority
// Manages bid and ask orders, handles order matching, and maintains order book state
// Features:
// - Price-time priority matching (best price first, then FIFO within price levels)
// - Support for Good-Till-Cancel (GTC) and Fill-and-Kill (FAK) orders
// - Order modification throuugh cancel-and-replace semantics
// - Real-time trade generation and order book level information
// - Efficient order lookup and management using hash maps and sorted containers
class OrderBook {
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
    bool CanMatch(Side side, Price price) const {
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

    // Matching engine that executes trades between compatible orders
    // Implements price-time priority and handles partial fills
    // Returns a vector of all trades executed during the matching process
    Trades MatchOrders() {
        Trades trades;
        trades.reserve(orders_.size());

        while (true) {
            if (bids_.empty() || asks_.empty()) {
                break;
            }
            auto& [bidPrice, bids] = *bids_.begin();
            auto& [askPrice, asks] = *asks_.begin();

            if (bidPrice < askPrice) {
                break;
            }

            while (bids.size() && asks.size()) {
                auto& bid = bids.front();
                auto& ask = asks.front();

                Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

                bid->Fill(quantity);
                ask->Fill(quantity);

                if (bid->IsFilled()) {
                    bids.pop_front();
                    orders_.erase(bid->GetOrderId());
                }

                if (ask->IsFilled()) {
                    asks.pop_front();
                    orders_.erase(ask->GetOrderId());
                }

                if (bids.empty()) {
                    bids_.erase(bidPrice);
                }

                if (asks.empty()) {
                    asks_.erase(askPrice);
                }

                trades.push_back(Trade{
                    TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity },
                    TradeInfo{ ask->GetOrderId(), ask->GetPrice(), quantity }
                    });
            }
        }

        // Handle Fill-and-Kill orders that couldn't be fully matched
        if (!bids_.empty()) {
            auto& [_, bids] = *bids_.begin();
            auto& order = bids.front();
            if (order->GetOrderType() == OrderType::FillAndKill) {
                CancelOrder(order->GetOrderId());
            }
        }
        if (!asks_.empty()) {
            auto& [_, asks] = *asks_.begin();
            auto& order = asks.front();
            if (order->GetOrderType() == OrderType::FillAndKill) {
                CancelOrder(order->GetOrderId());
            }
        }
        return trades;
    }

public:
    // Adds a new order to the order book and triggers matching
    // Returns any trades that result from the order addition
    Trades AddOrder(OrderPointer order) {
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

    // Removes an order from the order book without triggering matching
    // Handles cleanup of empty price levels
    void CancelOrder(OrderId orderId) {
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

    // Modifies an existing order using cancel-and-replace
    // Cancels the original order and adds a new one with updated parameters
    Trades MatchOrder(OrderModify order) {
        if (orders_.find(order.GetOrderId()) == orders_.end()) {
            return { };
        }
        const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
        CancelOrder(order.GetOrderId());
        return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
    }

    // Returns the total number of active orders in the book
    std::size_t Size() const { return orders_.size(); }

    // Gives a snapshot of current order book levels
    // Aggregates orders by price level and returns bid/ask information
    OrderBookLevelInfos GetOrderInfos() const {
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
};

int main() {
    OrderBook orderbook;
    const OrderId orderId = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 100, 10));
    std::cout << orderbook.Size() << std::endl; // 1
    orderbook.CancelOrder(orderId);
    std::cout << orderbook.Size() << std::endl; // 0
    return 0;
}
