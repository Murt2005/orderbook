#pragma once

#include "types.h"
#include <vector>


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
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade);

    const TradeInfo& GetBidTrade() const;
    const TradeInfo& GetAskTrade() const;

private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};

using Trades = std::vector<Trade>;
