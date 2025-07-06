#include "order_book.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <memory>


class OrderBookTests {

private:
    OrderBook orderbook;
    int testCount = 0;
    int passedTests = 0;

    void resetOrderBook() {
        orderbook = OrderBook();
    }

    void assertTrue(bool condition, const std::string& testName) {
        testCount++;
        if (condition) {
            passedTests++;
            std::cout << "Passed: " << testName << std::endl;
        } else {
            std::cout << "Failed: " << testName << std::endl;
        }
    }

    void printTestResults() {
        std::cout << "\n=== TEST RESULTS ===" << std::endl;
        std::cout << "Passed tests: " << passedTests << std::endl;
        if (passedTests == testCount) {
            std::cout << "Passed all tests" << std::endl;
        } else {
            std::cout << "Some tests failed" << std::endl;
        }
    }

public:
    void runAllTests() {
        std::cout << "Running OrderBook Tests..." << std::endl;

        testOrderAddition();
        testOrderCancellation();
        testSimpleMatching();
        testPartialFills();
        testPriceTimePriority();
        testGoodTillCancelOrders();
        testFillAndKillOrders();
        testOrderModification();
        testOrderBookLevels();
        testEdgeCases();

        printTestResults();
    }

    void testOrderAddition() {
        std::cout << "\n--- Test 1: Order Addition ---" << std::endl;
        resetOrderBook();

        auto buyOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        auto trades = orderbook.AddOrder(buyOrder);

        assertTrue(orderbook.Size() == 1, "OrderBook size after adding buy order");
        assertTrue(trades.empty(), "No trades on single order addition");

        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 105, 5);
        trades = orderbook.AddOrder(sellOrder);

        assertTrue(orderbook.Size() == 2, "OrderBook size after adding sell order");
        assertTrue(trades.empty(), "No trades when prices don't overlap");
    }

    void testOrderCancellation() {
        std::cout << "\n--- Test 2: Order Cancellation ---" << std::endl;
        resetOrderBook();

        auto order1 = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        auto order2 = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 105, 5);
        
        orderbook.AddOrder(order1);
        orderbook.AddOrder(order2);
        assertTrue(orderbook.Size() == 2, "Two orders added");

        orderbook.CancelOrder(1);
        assertTrue(orderbook.Size() == 1, "One order cancelled");

        orderbook.CancelOrder(999); 
        assertTrue(orderbook.Size() == 1, "Cancelling non-existent order doesn't affect size");

        orderbook.CancelOrder(2);
        assertTrue(orderbook.Size() == 0, "All orders cancelled");
    }

    void testSimpleMatching() {
        std::cout << "\n--- Test 3: Simple Matching ---" << std::endl;
        resetOrderBook();

        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Sell, 100, 10);
        auto trades = orderbook.AddOrder(sellOrder);
        assertTrue(trades.empty(), "No trades on first order");

        auto buyOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 100, 10);
        trades = orderbook.AddOrder(buyOrder);
        
        assertTrue(trades.size() == 1, "One trade executed");
        assertTrue(orderbook.Size() == 0, "Both orders filled and removed");
        
        if (!trades.empty()) {
            const auto& trade = trades[0];
            assertTrue(trade.GetBidTrade().quantity_ == 10, "Buy trade quantity correct");
            assertTrue(trade.GetAskTrade().quantity_ == 10, "Sell trade quantity correct");
        }
    }

    void testPartialFills() {
        std::cout << "\n--- Test 4: Partial Fills ---" << std::endl;
        resetOrderBook();

        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Sell, 100, 20);
        orderbook.AddOrder(sellOrder);

        auto buyOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 100, 10);
        auto trades = orderbook.AddOrder(buyOrder);

        assertTrue(trades.size() == 1, "One trade executed");
        assertTrue(orderbook.Size() == 1, "Sell order partially filled, still in book");

        auto levels = orderbook.GetOrderInfos();
        assertTrue(levels.GetAsks().size() == 1, "One ask level remaining");
        if (!levels.GetAsks().empty()) {
            assertTrue(levels.GetAsks()[0].quantity_ == 10, "Remaining quantity correct");
        }
    }

    void testPriceTimePriority() {
        std::cout << "\n--- Test 5: Price-Time Priority ---" << std::endl;
        resetOrderBook();

        auto buy1 = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 5);
        auto buy2 = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 100, 3);
        auto buy3 = std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Buy, 99, 10); // Lower price
        
        orderbook.AddOrder(buy1);
        orderbook.AddOrder(buy2);
        orderbook.AddOrder(buy3);

        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 4, Side::Sell, 100, 4);
        auto trades = orderbook.AddOrder(sellOrder);

        assertTrue(trades.size() == 1, "One trade executed");
        assertTrue(orderbook.Size() == 3, "Three orders remain");
        
        if (!trades.empty()) {
            assertTrue(trades[0].GetBidTrade().orderId_ == 1, "First order matched (time priority)");
            assertTrue(trades[0].GetBidTrade().quantity_ == 4, "Correct quantity matched");
        }
    }

    void testGoodTillCancelOrders() {
        std::cout << "\n--- Test 6: Good Till Cancel Orders ---" << std::endl;
        resetOrderBook();

        auto gtcOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        orderbook.AddOrder(gtcOrder);
        
        assertTrue(orderbook.Size() == 1, "GTC order added");
        
        auto nonMatchingSell = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 105, 5);
        orderbook.AddOrder(nonMatchingSell);
        
        assertTrue(orderbook.Size() == 2, "Both GTC orders remain when no match");
    }

    void testFillAndKillOrders() {
        std::cout << "\n--- Test 7: Fill And Kill Orders ---" << std::endl;
        resetOrderBook();

        auto fakOrder1 = std::make_shared<Order>(OrderType::FillAndKill, 1, Side::Buy, 100, 10);
        auto trades = orderbook.AddOrder(fakOrder1);
        
        assertTrue(orderbook.Size() == 0, "FAK order with no match rejected");
        assertTrue(trades.empty(), "No trades from rejected FAK order");

        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 100, 15);
        orderbook.AddOrder(sellOrder);

        auto fakOrder2 = std::make_shared<Order>(OrderType::FillAndKill, 3, Side::Buy, 100, 10);
        trades = orderbook.AddOrder(fakOrder2);
        
        assertTrue(trades.size() == 1, "FAK order executed");
        assertTrue(orderbook.Size() == 1, "Sell order partially filled, FAK order gone");

        if (!trades.empty()) {
            assertTrue(trades[0].GetBidTrade().quantity_ == 10, "Full FAK quantity traded");
            assertTrue(trades[0].GetAskTrade().quantity_ == 10, "Correct sell quantity traded");
        }

        auto fakOrder3 = std::make_shared<Order>(OrderType::FillAndKill, 4, Side::Buy, 100, 20);
        trades = orderbook.AddOrder(fakOrder3);
        
        assertTrue(trades.size() == 1, "Second FAK order partially executed");
        assertTrue(orderbook.Size() == 0, "Sell order filled, all FAK orders gone");

        auto sellOrder2 = std::make_shared<Order>(OrderType::GoodTillCancel, 5, Side::Sell, 100, 8);
        auto sellOrder3 = std::make_shared<Order>(OrderType::GoodTillCancel, 6, Side::Sell, 101, 6);
        auto sellOrder4 = std::make_shared<Order>(OrderType::GoodTillCancel, 7, Side::Sell, 102, 4);

        orderbook.AddOrder(sellOrder2);
        orderbook.AddOrder(sellOrder3);
        orderbook.AddOrder(sellOrder4);
        assertTrue(orderbook.Size() == 3, "Three sell orders added");

        auto fakOrder4 = std::make_shared<Order>(OrderType:: FillAndKill, 8, Side::Buy, 105, 15);
        trades = orderbook.AddOrder(fakOrder4);

        assertTrue(trades.size() == 3, "FAK order matched three price levels");
        assertTrue(orderbook.Size() == 1, "One sell order partially filled remains");

        if (trades.size() >= 3) {
            Quantity totalTraded = 0;
            for (const auto& trade : trades) {
                totalTraded += trade.GetBidTrade().quantity_;
            }
            assertTrue(totalTraded == 15, "Total fak quantity fully executed");
        }
    }

    void testOrderModification() {
        std::cout << "\n--- Test 8: Order Modification ---" << std::endl;
        resetOrderBook();

        auto originalOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        orderbook.AddOrder(originalOrder);

        OrderModify modification(1, Side::Buy, 105, 15);
        auto trades = orderbook.MatchOrder(modification);

        assertTrue(orderbook.Size() == 1, "Modified order in book");
        assertTrue(trades.empty(), "No trades from modification");

        auto levels = orderbook.GetOrderInfos();
        assertTrue(levels.GetBids().size() == 1, "One bid level");
        if (!levels.GetBids().empty()) {
            assertTrue(levels.GetBids()[0].price_ == 105, "Price modified correctly");
            assertTrue(levels.GetBids()[0].quantity_ == 15, "Quantity modified correctly");
        }
    }

    void testOrderBookLevels() {
        std::cout << "\n--- Test 9: OrderBook Levels ---" << std::endl;
        resetOrderBook();

        orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
        orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 99, 5));
        orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Sell, 101, 8));
        orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 4, Side::Sell, 102, 12));

        auto levels = orderbook.GetOrderInfos();

        assertTrue(levels.GetBids().size() == 2, "Two bid levels");
        assertTrue(levels.GetAsks().size() == 2, "Two ask levels");

        if (levels.GetBids().size() >= 2) {
            assertTrue(levels.GetBids()[0].price_ > levels.GetBids()[1].price_, "Bids ordered high to low");
        }

        if (levels.GetAsks().size() >= 2) {
            assertTrue(levels.GetAsks()[0].price_ < levels.GetAsks()[1].price_, "Asks ordered low to high");
        }
    }

    void testEdgeCases() {
        std::cout << "\n--- Test 10: Edge Cases ---" << std::endl;
        resetOrderBook();

        auto order1 = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        auto order2 = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Sell, 95, 5);
        
        orderbook.AddOrder(order1);
        auto trades = orderbook.AddOrder(order2);
        
        assertTrue(orderbook.Size() == 1, "Duplicate order ID rejected");
        assertTrue(trades.empty(), "No trades from duplicate order");

        auto largeOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 100, UINT32_MAX);
        orderbook.AddOrder(largeOrder);
        assertTrue(orderbook.Size() == 2, "Large quantity order accepted");
    }
};

int main() {
    OrderBookTests tests;
    tests.runAllTests();

    return 0;
}
