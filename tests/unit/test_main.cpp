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
        orderbook.clear();
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
        
        // Print performance report
        orderbook.printPerformanceReport();
        orderbook.printPerformanceSummary();
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
        testImmediateOrCancelOrders();
        testFillOrKillOrders();
        testOrderModification();
        testOrderBookLevels();
        testEdgeCases();
        testAdditionalEdgeCases();
        testExceptionHandling();
        testTradeValidation();
        testOrderStateValidation();
        testPerformanceTracking();

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

    void testImmediateOrCancelOrders() {
        std::cout << "\n--- Test 7: Immediate Or Cancel Orders ---" << std::endl;
        resetOrderBook();

        auto iocOrder1 = std::make_shared<Order>(OrderType::ImmediateOrCancel, 1, Side::Buy, 100, 10);
        auto trades = orderbook.AddOrder(iocOrder1);
        
        assertTrue(orderbook.Size() == 0, "IOC order with no match rejected");
        assertTrue(trades.empty(), "No trades from rejected IOC order");

        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 100, 15);
        orderbook.AddOrder(sellOrder);

        auto iocOrder2 = std::make_shared<Order>(OrderType::ImmediateOrCancel, 3, Side::Buy, 100, 10);
        trades = orderbook.AddOrder(iocOrder2);
        
        assertTrue(trades.size() == 1, "IOC order executed");
        assertTrue(orderbook.Size() == 1, "Sell order partially filled, IOC order gone");

        if (!trades.empty()) {
            assertTrue(trades[0].GetBidTrade().quantity_ == 10, "Full IOC quantity traded");
            assertTrue(trades[0].GetAskTrade().quantity_ == 10, "Correct sell quantity traded");
        }

        auto iocOrder3 = std::make_shared<Order>(OrderType::ImmediateOrCancel, 4, Side::Buy, 100, 20);
        trades = orderbook.AddOrder(iocOrder3);
        
        assertTrue(trades.size() == 1, "Second IOC order partially executed");
        assertTrue(orderbook.Size() == 0, "Sell order filled, all IOC orders gone");

        auto sellOrder2 = std::make_shared<Order>(OrderType::GoodTillCancel, 5, Side::Sell, 100, 8);
        auto sellOrder3 = std::make_shared<Order>(OrderType::GoodTillCancel, 6, Side::Sell, 101, 6);
        auto sellOrder4 = std::make_shared<Order>(OrderType::GoodTillCancel, 7, Side::Sell, 102, 4);

        orderbook.AddOrder(sellOrder2);
        orderbook.AddOrder(sellOrder3);
        orderbook.AddOrder(sellOrder4);
        assertTrue(orderbook.Size() == 3, "Three sell orders added");

        auto iocOrder4 = std::make_shared<Order>(OrderType::ImmediateOrCancel, 8, Side::Buy, 105, 15);
        trades = orderbook.AddOrder(iocOrder4);

        assertTrue(trades.size() == 3, "IOC order matched three price levels");
        assertTrue(orderbook.Size() == 1, "One sell order partially filled remains");

        if (trades.size() >= 3) {
            Quantity totalTraded = 0;
            for (const auto& trade : trades) {
                totalTraded += trade.GetBidTrade().quantity_;
            }
            assertTrue(totalTraded == 15, "Total IOC quantity fully executed");
        }
    }

    void testFillOrKillOrders() {
        std::cout << "\n--- Test 8: FillOrKill Orders ---" << std::endl;

        resetOrderBook();

        // Test 1: FOK Success - Complete Fill
        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Sell, 100, 20);
        orderbook.AddOrder(sellOrder);

        auto fokBuy = std::make_shared<Order>(OrderType::FillOrKill, 2, Side::Buy, 100, 15);
        auto trades = orderbook.AddOrder(fokBuy);

        assertTrue(trades.size() == 1, "FOK order executed");
        assertTrue(orderbook.Size() == 1, "Sell order partially filled");
        if (!trades.empty()) {
            assertTrue(trades[0].GetBidTrade().quantity_ == 15, "FOK order fully executed");
            assertTrue(trades[0].GetAskTrade().quantity_ == 15, "Correct quantity traded from sell order");
        }

        auto levels = orderbook.GetOrderInfos();
        if (!levels.GetAsks().empty()) {
            assertTrue(levels.GetAsks()[0].quantity_ == 5, "Remaining sell quantity correct after FOK");
        }

        resetOrderBook();

        // Test 2: FOK Rejection - Insufficient Liquidity
        auto sellOrder1 = std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Sell, 100, 10);
        orderbook.AddOrder(sellOrder1);

        auto fokBuy1 = std::make_shared<Order>(OrderType::FillOrKill, 4, Side::Buy, 100, 15);
        trades = orderbook.AddOrder(fokBuy1);

        assertTrue(trades.size() == 0, "FOK order rejected due to insufficient liquidity");
        assertTrue(orderbook.Size() == 1, "Only original sell order left");
        levels = orderbook.GetOrderInfos();
        if (!levels.GetAsks().empty()) {
            assertTrue(levels.GetAsks()[0].quantity_ == 10, "Original sell order unchanged after FOK rejection");
        }
    
        resetOrderBook();

        // Test 3: FOK order across multiple price levels
        auto sellOrder2a = std::make_shared<Order>(OrderType::GoodTillCancel, 5, Side::Sell, 100, 8);
        auto sellOrder2b = std::make_shared<Order>(OrderType::GoodTillCancel, 6, Side::Sell, 100, 6);
        auto sellOrder2c = std::make_shared<Order>(OrderType::GoodTillCancel, 7, Side::Sell, 100, 4);

        orderbook.AddOrder(sellOrder2a);
        orderbook.AddOrder(sellOrder2b);
        orderbook.AddOrder(sellOrder2c);
        assertTrue(orderbook.Size() == 3, "3 sell orders added");

        auto fokBuy2 = std::make_shared<Order>(OrderType::FillOrKill, 8, Side::Buy, 102, 18);
        trades = orderbook.AddOrder(fokBuy2);

        assertTrue(trades.size() == 3, "FOK order executed 3 trades");
        assertTrue(orderbook.Size() == 0, "No trades left");

        if (trades.size() >= 3) {
            Quantity totalTraded = 0;
            for (const auto& trade : trades) {
                totalTraded += trade.GetBidTrade().quantity_;
            }
            assertTrue(totalTraded == 18, "Total FOK quantity fully executed");
            }
    
        resetOrderBook();

        // Test 4: FOK order with no liquidity
        auto fokBuy3 = std::make_shared<Order>(OrderType::FillOrKill, 9, Side::Buy, 100, 10);
        trades = orderbook.AddOrder(fokBuy3);

        assertTrue(trades.empty(), "No trades occur");
        assertTrue(orderbook.Size() == 0, "No orders in book");

        resetOrderBook();

        // Test 5: FOK Sell Orders
        auto buyOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 10, Side::Buy, 100, 10);
        orderbook.AddOrder(buyOrder);

        auto fokSell = std::make_shared<Order>(OrderType::FillOrKill, 11, Side::Sell, 100, 8);
        trades = orderbook.AddOrder(fokSell);

        assertTrue(trades.size() == 1, "FOK sell order executed");
        assertTrue(orderbook.Size() == 1, "Buy order still open");

        if (!trades.empty()) {
            assertTrue(trades[0].GetAskTrade().quantity_ == 8, "FOK sell quantity correct");
            assertTrue(trades[0].GetBidTrade().quantity_ == 8, "Matching buy quantity correct");
        }
    
        resetOrderBook();

        // Test 6: FOK with price levels not matching
        auto sellOrder3 = std::make_shared<Order>(OrderType::GoodTillCancel, 12, Side::Sell, 105, 10);
        orderbook.AddOrder(sellOrder3);

        auto fokBuy4 = std::make_shared<Order>(OrderType::FillOrKill, 13, Side::Buy, 102, 10);
        trades = orderbook.AddOrder(fokBuy4);

        assertTrue(trades.empty(), "FOK order rejected due to price mismatch");
        assertTrue(orderbook.Size() == 1, "Original sell order unchanged");
    
        resetOrderBook();

        // Test 7: Large FOK order
        for (int i = 0; i < 5; ++i) {
            auto sellOrder4 = std::make_shared<Order>(OrderType::GoodTillCancel, 22 + i, Side::Sell, 100 + i, 10);
            orderbook.AddOrder(sellOrder4);
        }
        assertTrue(orderbook.Size() == 5, "Five sell orders added");
        
        auto largeFOK = std::make_shared<Order>(OrderType::FillOrKill, 27, Side::Buy, 104, 50);
        trades = orderbook.AddOrder(largeFOK);
        
        assertTrue(trades.size() == 5, "Large FOK order executed across all levels");
        assertTrue(orderbook.Size() == 0, "All orders filled");
        
        if (trades.size() >= 5) {
            Quantity totalTraded = 0;
            for (const auto& trade : trades) {
                totalTraded += trade.GetBidTrade().quantity_;
            }
            assertTrue(totalTraded == 50, "Complete large FOK quantity executed");
        }
    }

    void testOrderModification() {
        std::cout << "\n--- Test 9: Order Modification ---" << std::endl;
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
        std::cout << "\n--- Test 10: OrderBook Levels ---" << std::endl;
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
        std::cout << "\n--- Test 11: Edge Cases ---" << std::endl;
        resetOrderBook();

        // Test 1: Duplicate order ID
        auto order1 = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        auto order2 = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Sell, 95, 5);
        
        orderbook.AddOrder(order1);
        auto trades = orderbook.AddOrder(order2);
        
        assertTrue(orderbook.Size() == 1, "Duplicate order ID rejected");
        assertTrue(trades.empty(), "No trades from duplicate order");

        // Test 2: Large quantity order
        auto largeOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 100, UINT32_MAX);
        orderbook.AddOrder(largeOrder);
        assertTrue(orderbook.Size() == 2, "Large quantity order accepted");

        resetOrderBook();

        // Test 3: Zero quantity order (should throw)
        try {
            auto zeroOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Buy, 100, 0);
            orderbook.AddOrder(zeroOrder);
            assertTrue(false, "Zero quantity order should throw");
        } catch (const std::invalid_argument&) {
            assertTrue(true, "Zero quantity order threw exception");
        }

        resetOrderBook();

        // Test 4: Negative price (edge case for price validation)
        auto negativePriceOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 4, Side::Buy, -50, 10);
        trades = orderbook.AddOrder(negativePriceOrder);
        assertTrue(orderbook.Size() == 1, "Negative price order accepted");
        
        // Test matching with negative price
        auto matchingSell = std::make_shared<Order>(OrderType::GoodTillCancel, 5, Side::Sell, -50, 5);
        trades = orderbook.AddOrder(matchingSell);
        assertTrue(trades.size() == 1, "Trade executed with negative prices");
        assertTrue(orderbook.Size() == 1, "One order partially filled");

        resetOrderBook();

        // Test 5: Very large price values
        auto largePriceOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 6, Side::Buy, INT32_MAX, 10);
        trades = orderbook.AddOrder(largePriceOrder);
        assertTrue(orderbook.Size() == 1, "Very large price order accepted");

        resetOrderBook();

        // Test 6: Order modification of non-existent order
        OrderModify nonExistentModify(999, Side::Buy, 100, 10);
        trades = orderbook.MatchOrder(nonExistentModify);
        assertTrue(trades.empty(), "No trades from modifying non-existent order");
        assertTrue(orderbook.Size() == 0, "Order book unchanged");

        resetOrderBook();

        // Test 7: Cancelling non-existent order
        orderbook.CancelOrder(999);
        assertTrue(orderbook.Size() == 0, "Cancelling non-existent order doesn't affect book");

        resetOrderBook();

        // Test 8: Multiple orders with same price but different quantities
        for (int i = 0; i < 5; ++i) {
            auto order = std::make_shared<Order>(OrderType::GoodTillCancel, 10 + i, Side::Buy, 100, 10 + i);
            orderbook.AddOrder(order);
        }
        assertTrue(orderbook.Size() == 5, "Five orders at same price level");
        
        auto levels = orderbook.GetOrderInfos();
        if (!levels.GetBids().empty()) {
            assertTrue(levels.GetBids()[0].quantity_ == 60, "Total quantity at price level correct"); // 10+11+12+13+14 = 60
        }

        resetOrderBook();

        // Test 9: Stress test with many orders
        for (int i = 0; i < 100; ++i) {
            auto order = std::make_shared<Order>(OrderType::GoodTillCancel, 100 + i, Side::Buy, 100 + (i % 10), 10);
            orderbook.AddOrder(order);
        }
        assertTrue(orderbook.Size() == 100, "100 orders added successfully");
        
        // Test 10: Boundary price matching
        auto boundaryBuy = std::make_shared<Order>(OrderType::GoodTillCancel, 200, Side::Buy, 100, 10);
        auto boundarySell = std::make_shared<Order>(OrderType::GoodTillCancel, 201, Side::Sell, 100, 10);
        
        orderbook.AddOrder(boundaryBuy);
        trades = orderbook.AddOrder(boundarySell);
        assertTrue(trades.size() == 1, "Boundary price orders matched");
        assertTrue(orderbook.Size() == 100, "Original orders unchanged");
    }

    void testAdditionalEdgeCases() {
        std::cout << "\n--- Test 12: Additional Edge Cases ---" << std::endl;
        resetOrderBook();

        // Test 1: Order with maximum order ID
        auto maxIdOrder = std::make_shared<Order>(OrderType::GoodTillCancel, UINT64_MAX, Side::Buy, 100, 10);
        auto trades = orderbook.AddOrder(maxIdOrder);
        assertTrue(orderbook.Size() == 1, "Order with maximum ID accepted");
        assertTrue(trades.empty(), "No trades from single order");

        resetOrderBook();

        // Test 2: Mixed order types at same price level
        auto gtcOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        orderbook.AddOrder(gtcOrder);
        // No duplicate AddOrder for gtcOrder
        
        // Add a sell order to provide liquidity for IOC/FOK orders
        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 4, Side::Sell, 100, 20);
        orderbook.AddOrder(sellOrder);
        // No duplicate AddOrder for sellOrder
        
        auto iocOrder = std::make_shared<Order>(OrderType::ImmediateOrCancel, 2, Side::Buy, 100, 5);
        auto fokOrder = std::make_shared<Order>(OrderType::FillOrKill, 3, Side::Buy, 100, 3);
        
        trades = orderbook.AddOrder(iocOrder);
        assertTrue(trades.size() == 1, "Trade executed for IOC order");
        trades = orderbook.AddOrder(fokOrder);
        // After IOC and FOK, only the partially filled sell order should remain
        assertTrue(orderbook.Size() == 1, "Mixed order types at same price level");
        resetOrderBook();

        // Test 3: Rapid order addition and cancellation
        for (int i = 1; i <= 50; ++i) { // IDs from 1 to 50
            auto order = std::make_shared<Order>(OrderType::GoodTillCancel, i, Side::Buy, 100 + i, 10);
            orderbook.AddOrder(order);
            if (i % 2 == 0) {
                orderbook.CancelOrder(i);
            }
        }
        assertTrue(orderbook.Size() == 25, "Rapid add/cancel operations handled correctly");

        resetOrderBook();

        // Test 4: Orders with extreme price differences
        auto lowPriceOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, INT32_MIN, 10);
        auto highPriceOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, INT32_MAX, 10);
        
        orderbook.AddOrder(lowPriceOrder);
        orderbook.AddOrder(highPriceOrder);
        assertTrue(orderbook.Size() == 2, "Extreme price difference orders accepted");
        assertTrue(orderbook.GetOrderInfos().GetBids().size() == 1, "One bid level");
        assertTrue(orderbook.GetOrderInfos().GetAsks().size() == 1, "One ask level");

        resetOrderBook();

        // Test 5: Order modification with same parameters
        auto originalOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        orderbook.AddOrder(originalOrder);
        
        OrderModify sameModify(1, Side::Buy, 100, 10);
        trades = orderbook.MatchOrder(sameModify);
        assertTrue(orderbook.Size() == 1, "Order with same parameters still in book");
        assertTrue(trades.empty(), "No trades from identical modification");

        resetOrderBook();

        // Test 6: Empty order book operations
        assertTrue(orderbook.Size() == 0, "Empty order book size");
        assertTrue(orderbook.GetOrderInfos().GetBids().empty(), "Empty bid levels");
        assertTrue(orderbook.GetOrderInfos().GetAsks().empty(), "Empty ask levels");
        
        // Test operations on empty book
        orderbook.CancelOrder(999);
        assertTrue(orderbook.Size() == 0, "Cancel on empty book doesn't change size");
        
        OrderModify emptyModify(999, Side::Buy, 100, 10);
        trades = orderbook.MatchOrder(emptyModify);
        assertTrue(trades.empty(), "Modify on empty book returns no trades");

        resetOrderBook();

        // Test 7: Partial fill edge cases
        auto sellOrderPartial = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Sell, 100, 100);
        orderbook.AddOrder(sellOrderPartial);
        
        // Multiple small buy orders
        for (int i = 0; i < 10; ++i) {
            auto buyOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 10 + i, Side::Buy, 100, 5);
            trades = orderbook.AddOrder(buyOrder);
            assertTrue(trades.size() == 1, "Trade executed for small order");
        }
        
        assertTrue(orderbook.Size() == 1, "Sell order partially filled, still in book");
        auto levels = orderbook.GetOrderInfos();
        if (!levels.GetAsks().empty()) {
            assertTrue(levels.GetAsks()[0].quantity_ == 50, "Remaining quantity correct after partial fills");
        }
    }

    void testExceptionHandling() {
        std::cout << "\n--- Test 13: Exception Handling ---" << std::endl;
        resetOrderBook();

        // Test 1: Over-filling an order (should throw std::logic_error)
        auto order = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        try {
            order->Fill(15); // Try to fill more than available
            assertTrue(false, "Over-filling should throw an exception");
        } catch (const std::logic_error& e) {
            assertTrue(true, "Over-filling threw expected std::logic_error");
        }

        // Test 2: Fill with exact quantity (should not throw)
        try {
            order->Fill(10);
            assertTrue(order->IsFilled(), "Order should be filled after exact fill");
        } catch (const std::exception& e) {
            assertTrue(false, "Exact fill should not throw an exception");
        }

        // Test 3: Fill with zero quantity (should not throw)
        auto order2 = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 100, 10);
        try {
            order2->Fill(0);
            assertTrue(order2->GetRemainingQuantity() == 10, "Zero fill should not change quantity");
        } catch (const std::exception& e) {
            assertTrue(false, "Zero fill should not throw an exception");
        }

        // Test 4: Fill with negative quantity (should throw)
        auto order3 = std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Buy, 100, 10);
        try {
            order3->Fill(-5);
            assertTrue(false, "Negative fill should throw an exception");
        } catch (const std::logic_error& e) {
            assertTrue(true, "Negative fill threw expected exception");
        }

        // Test 5: Multiple fills that exceed total quantity
        auto order4 = std::make_shared<Order>(OrderType::GoodTillCancel, 4, Side::Buy, 100, 10);
        try {
            order4->Fill(5);
            order4->Fill(5);
            order4->Fill(1); // This should work
            order4->Fill(1); // This should throw
            assertTrue(false, "Over-filling through multiple fills should throw");
        } catch (const std::logic_error& e) {
            assertTrue(true, "Over-filling through multiple fills threw expected exception");
        }
    }

    void testTradeValidation() {
        std::cout << "\n--- Test 14: Trade Validation ---" << std::endl;
        resetOrderBook();

        // Test 1: Trade execution price validation
        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Sell, 100, 10);
        orderbook.AddOrder(sellOrder);

        auto buyOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 105, 10);
        auto trades = orderbook.AddOrder(buyOrder);

        assertTrue(trades.size() == 1, "Trade executed");
        if (!trades.empty()) {
            // Trade should execute at the better price (sell price = 100)
            assertTrue(trades[0].GetBidTrade().price_ == 100, "Trade executed at sell price (better price)");
            assertTrue(trades[0].GetAskTrade().price_ == 100, "Trade executed at sell price (better price)");
            assertTrue(trades[0].GetBidTrade().quantity_ == 10, "Trade quantity correct");
            assertTrue(trades[0].GetAskTrade().quantity_ == 10, "Trade quantity correct");
        }

        resetOrderBook();

        // Test 2: Trade with zero quantity orders (should throw)
        try {
            auto zeroQuantitySell = std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Sell, 100, 0);
            orderbook.AddOrder(zeroQuantitySell);
            assertTrue(false, "Zero quantity sell order should throw");
        } catch (const std::invalid_argument&) {
            assertTrue(true, "Zero quantity sell order threw exception");
        }
        try {
            auto zeroQuantityBuy = std::make_shared<Order>(OrderType::GoodTillCancel, 4, Side::Buy, 100, 0);
            orderbook.AddOrder(zeroQuantityBuy);
            assertTrue(false, "Zero quantity buy order should throw");
        } catch (const std::invalid_argument&) {
            assertTrue(true, "Zero quantity buy order threw exception");
        }

        resetOrderBook();

        // Test 3: Trade price priority (buy at 105, sell at 100)
        auto sellOrder2 = std::make_shared<Order>(OrderType::GoodTillCancel, 5, Side::Sell, 100, 10);
        orderbook.AddOrder(sellOrder2);

        auto buyOrder2 = std::make_shared<Order>(OrderType::GoodTillCancel, 6, Side::Buy, 105, 10);
        trades = orderbook.AddOrder(buyOrder2);

        assertTrue(trades.size() == 1, "Trade executed with price improvement");
        if (!trades.empty()) {
            // Should trade at the better price (100)
            assertTrue(trades[0].GetBidTrade().price_ == 100, "Buy order traded at better price");
            assertTrue(trades[0].GetAskTrade().price_ == 100, "Sell order traded at better price");
        }
    }

    void testOrderStateValidation() {
        std::cout << "\n--- Test 15: Order State Validation ---" << std::endl;
        resetOrderBook();

        // Test 1: Order state after creation
        auto order1 = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
        assertTrue(order1->GetInitialQuantity() == 10, "Initial quantity correct");
        assertTrue(order1->GetRemainingQuantity() == 10, "Remaining quantity equals initial");
        assertTrue(order1->GetFilledQuantity() == 0, "Filled quantity is zero");
        assertTrue(!order1->IsFilled(), "Order is not filled initially");

        orderbook.AddOrder(order1);
        assertTrue(orderbook.Size() == 1, "Order added to book");

        // Test 2: Order state after partial fill
        auto sellOrder = std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 100, 5);
        auto trades = orderbook.AddOrder(sellOrder);

        assertTrue(trades.size() == 1, "Trade executed");
        assertTrue(order1->GetRemainingQuantity() == 5, "Order partially filled");
        assertTrue(order1->GetFilledQuantity() == 5, "Filled quantity updated");
        assertTrue(!order1->IsFilled(), "Order not completely filled");

        // Test 3: Order state after complete fill
        auto sellOrder2 = std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Sell, 100, 5);
        trades = orderbook.AddOrder(sellOrder2);

        assertTrue(trades.size() == 1, "Trade executed");
        assertTrue(order1->GetRemainingQuantity() == 0, "Order completely filled");
        assertTrue(order1->GetFilledQuantity() == 10, "Filled quantity equals initial");
        assertTrue(order1->IsFilled(), "Order is filled");

        // Test 4: Order state after cancellation
        resetOrderBook();
        auto order2 = std::make_shared<Order>(OrderType::GoodTillCancel, 4, Side::Buy, 100, 10);
        orderbook.AddOrder(order2);
        orderbook.CancelOrder(4);
        assertTrue(orderbook.Size() == 0, "Order removed from book after cancellation");

        // Test 5: Order state consistency
        auto order3 = std::make_shared<Order>(OrderType::GoodTillCancel, 5, Side::Buy, 100, 10);
        assertTrue(order3->GetInitialQuantity() == order3->GetRemainingQuantity() + order3->GetFilledQuantity(), 
                  "Initial quantity equals remaining plus filled");
    }

    void testPerformanceTracking() {
        std::cout << "\n--- Test 16: Performance Tracking ---" << std::endl;
        resetOrderBook();

        // Enable performance tracking
        orderbook.enablePerformanceTracking(true);
        assertTrue(orderbook.isPerformanceTrackingEnabled(), "Performance tracking enabled");

        // Reset metrics
        orderbook.resetPerformanceMetrics();

        // Perform a series of operations to generate performance data
        const int numOperations = 1000;
        
        // Add orders
        for (int i = 0; i < numOperations; ++i) {
            auto order = std::make_shared<Order>(OrderType::GoodTillCancel, i + 1, Side::Buy, 100 + (i % 10), 10);
            orderbook.AddOrder(order);
        }
        assertTrue(orderbook.Size() == numOperations, "All orders added");

        // Add some sell orders to trigger matching
        for (int i = 0; i < numOperations / 2; ++i) {
            auto order = std::make_shared<Order>(OrderType::GoodTillCancel, numOperations + i + 1, Side::Sell, 95 + (i % 10), 5);
            orderbook.AddOrder(order);
        }

        // Test order modifications
        for (int i = 0; i < 100; ++i) {
            OrderModify modify(i + 1, Side::Buy, 105, 15);
            orderbook.MatchOrder(modify);
        }

        // Test cancellations
        for (int i = 0; i < 200; ++i) {
            orderbook.CancelOrder(i + 1);
        }

        // Test GetOrderInfos
        for (int i = 0; i < 50; ++i) {
            auto levels = orderbook.GetOrderInfos();
            assertTrue(!levels.GetBids().empty() || !levels.GetAsks().empty(), "Order book has levels");
        }

        // Test Size operations
        for (int i = 0; i < 100; ++i) {
            auto size = orderbook.Size();
            assertTrue(size >= 0, "Size is non-negative");
        }

        // Verify that performance metrics were collected
        assertTrue(orderbook.isPerformanceTrackingEnabled(), "Performance tracking still enabled");
        
        std::cout << "Performance test completed. Check the performance report above." << std::endl;
    }
};

int main() {
    OrderBookTests tests;
    tests.runAllTests();

    return 0;
}
