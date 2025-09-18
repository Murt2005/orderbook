#include "order_book.h"
#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>

class PerformanceBenchmark {
private:
    OrderBook orderbook_;
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> priceDist_;
    std::uniform_int_distribution<> quantityDist_;
    std::uniform_int_distribution<> sideDist_;
    
public:
    PerformanceBenchmark() 
        : gen_(rd_()), 
          priceDist_(100, 200), 
          quantityDist_(1, 100),
          sideDist_(0, 1) {
        
        orderbook_.enablePerformanceTracking(true);
        orderbook_.resetPerformanceMetrics();
    }
    
    void runBenchmark() {
        std::cout << "=== ORDER BOOK PERFORMANCE BENCHMARK ===" << std::endl;
        std::cout << "Running various workloads to test performance..." << std::endl << std::endl;
        
        testHighFrequencyOrders();
        
        testOrderMatching();
        
        testMixedOperations();
        
        testLargeOrderBook();
        
        std::cout << "\n=== FINAL PERFORMANCE REPORT ===" << std::endl;
        orderbook_.printPerformanceReport();
        orderbook_.printPerformanceSummary();
    }
    
private:
    void testHighFrequencyOrders() {
        std::cout << "Test 1: High-frequency order additions (10,000 orders)" << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 10000; ++i) {
            Side side = (sideDist_(gen_) == 0) ? Side::Buy : Side::Sell;
            auto order = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                i + 1,
                side,
                priceDist_(gen_),
                quantityDist_(gen_)
            );
            orderbook_.AddOrder(order);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "  Completed in " << duration.count() << " ms" << std::endl;
        std::cout << "  Orders per second: " << (10000 * 1000) / duration.count() << std::endl;
        std::cout << "  Order book size: " << orderbook_.Size() << std::endl << std::endl;
    }
    
    void testOrderMatching() {
        std::cout << "Test 2: Order matching stress test" << std::endl;
        
        // Clear order book for clean test
        orderbook_.clear();
        orderbook_.resetPerformanceMetrics();
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 5000; ++i) {
            auto buyOrder = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                i + 1,
                Side::Buy,
                100 + (i % 20),
                10
            );
            orderbook_.AddOrder(buyOrder);
        }
        
        for (int i = 0; i < 5000; ++i) {
            auto sellOrder = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                5000 + i + 1,
                Side::Sell,
                95 + (i % 25),
                5
            );
            orderbook_.AddOrder(sellOrder);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "  Completed in " << duration.count() << " ms" << std::endl;
        std::cout << "  Orders per second: " << (10000 * 1000) / duration.count() << std::endl;
        std::cout << "  Final order book size: " << orderbook_.Size() << std::endl << std::endl;
    }
    
    void testMixedOperations() {
        std::cout << "Test 3: Mixed operations (add, modify, cancel)" << std::endl;
        
        // Clear order book for clean test
        orderbook_.clear();
        orderbook_.resetPerformanceMetrics();
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 2000; ++i) {
            auto order = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                i + 1,
                Side::Buy,
                priceDist_(gen_),
                quantityDist_(gen_)
            );
            orderbook_.AddOrder(order);
        }
        
        for (int i = 0; i < 500; ++i) {
            OrderModify modify(i + 1, Side::Buy, priceDist_(gen_), quantityDist_(gen_));
            orderbook_.MatchOrder(modify);
        }
        
        for (int i = 0; i < 300; ++i) {
            orderbook_.CancelOrder(i + 1);
        }
        
        for (int i = 0; i < 100; ++i) {
            auto levels = orderbook_.GetOrderInfos();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "  Completed in " << duration.count() << " ms" << std::endl;
        std::cout << "  Operations per second: " << (2900 * 1000) / duration.count() << std::endl;
        std::cout << "  Final order book size: " << orderbook_.Size() << std::endl << std::endl;
    }
    
    void testLargeOrderBook() {
        std::cout << "Test 4: Large order book operations" << std::endl;
        
        // Clear order book for clean test
        orderbook_.clear();
        orderbook_.resetPerformanceMetrics();
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 50000; ++i) {
            Side side = (sideDist_(gen_) == 0) ? Side::Buy : Side::Sell;
            auto order = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                i + 1,
                side,
                50 + (i % 100),
                quantityDist_(gen_)
            );
            orderbook_.AddOrder(order);
        }
        
        auto levels = orderbook_.GetOrderInfos();
        auto size = orderbook_.Size();
        
        for (int i = 0; i < 1000; ++i) {
            orderbook_.CancelOrder(i + 1);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "  Completed in " << duration.count() << " ms" << std::endl;
        std::cout << "  Orders per second: " << (51000 * 1000) / duration.count() << std::endl;
        std::cout << "  Final order book size: " << orderbook_.Size() << std::endl;
        std::cout << "  Price levels (bids): " << levels.GetBids().size() << std::endl;
        std::cout << "  Price levels (asks): " << levels.GetAsks().size() << std::endl << std::endl;
    }
};

int main() {
    PerformanceBenchmark benchmark;
    benchmark.runBenchmark();
    
    return 0;
} 
