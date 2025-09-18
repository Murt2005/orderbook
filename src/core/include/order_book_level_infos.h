#pragma once

#include "types.h"
#include <vector>

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
    OrderBookLevelInfos(const LevelInfos& bids, const LevelInfos& asks);

    const LevelInfos& GetBids() const;
    const LevelInfos& GetAsks() const;

private:
    LevelInfos bids_;
    LevelInfos asks_;
};
