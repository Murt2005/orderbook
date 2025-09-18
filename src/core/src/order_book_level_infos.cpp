#include "order_book_level_infos.h"

OrderBookLevelInfos::OrderBookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
    : bids_{ bids }
    , asks_{ asks }
{ }

const LevelInfos& OrderBookLevelInfos::GetBids() const {
    return bids_;
}

const LevelInfos& OrderBookLevelInfos::GetAsks() const {
    return asks_;
}
