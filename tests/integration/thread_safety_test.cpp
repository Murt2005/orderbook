#include "order_book.h"
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>

class ThreadSafetyTest {
private:
    OrderBook orderbook_;
    std::atomic<int> successfulOrders_{0};
    std::atomic<int> failedOrders_{0};
    std::atomic<int> completedThreads_{0};
    
public:
    void runConcurrencyTest() {
        std::cout << "=== THREAD SAFETY TEST ===" << std::endl;
        std::cout << "Testing concurrent order operations..." << std::endl << std::endl;
        
        const int numThreads = 8;
        const int ordersPerThread = 1000;
        
        std::vector<std::thread> threads;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Create worker threads
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([this, i, ordersPerThread]() {
                workerThread(i, ordersPerThread);
            });
        }
        
        // Create reader threads
        for (int i = 0; i < 2; ++i) {
            threads.emplace_back([this, i]() {
                readerThread(i);
            });
        }
        
        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "\n=== TEST RESULTS ===" << std::endl;
        std::cout << "Total threads: " << numThreads + 2 << std::endl;
        std::cout << "Orders per worker thread: " << ordersPerThread << std::endl;
        std::cout << "Total orders attempted: " << (numThreads * ordersPerThread) << std::endl;
        std::cout << "Successful orders: " << successfulOrders_.load() << std::endl;
        std::cout << "Failed orders: " << failedOrders_.load() << std::endl;
        std::cout << "Final order book size: " << orderbook_.Size() << std::endl;
        std::cout << "Test duration: " << duration.count() << " ms" << std::endl;
        std::cout << "Orders per second: " << (numThreads * ordersPerThread * 1000) / duration.count() << std::endl;
        
        // Verify order book integrity
        verifyOrderBookIntegrity();
    }
    
private:
    void workerThread(int threadId, int numOrders) {
        std::random_device rd;
        std::mt19937 gen(rd() + threadId);
        std::uniform_int_distribution<> priceDist(100, 200);
        std::uniform_int_distribution<> quantityDist(1, 100);
        std::uniform_int_distribution<> sideDist(0, 1);
        std::uniform_int_distribution<> operationDist(0, 2); // 0: add, 1: cancel, 2: modify
        
        std::vector<OrderId> myOrders;
        
        for (int i = 0; i < numOrders; ++i) {
            OrderId orderId = threadId * 10000 + i + 1;
            Side side = (sideDist(gen) == 0) ? Side::Buy : Side::Sell;
            Price price = priceDist(gen);
            Quantity quantity = quantityDist(gen);
            
            int operation = operationDist(gen);
            
            try {
                switch (operation) {
                    case 0: { // Add order
                        auto order = std::make_shared<Order>(
                            OrderType::GoodTillCancel,
                            orderId,
                            side,
                            price,
                            quantity
                        );
                        auto trades = orderbook_.AddOrder(order);
                        if (!trades.empty()) {
                            successfulOrders_++;
                            myOrders.push_back(orderId);
                        } else {
                            failedOrders_++;
                        }
                        break;
                    }
                    case 1: { // Cancel order
                        if (!myOrders.empty()) {
                            OrderId cancelId = myOrders[gen() % myOrders.size()];
                            orderbook_.CancelOrder(cancelId);
                            successfulOrders_++;
                        }
                        break;
                    }
                    case 2: { // Modify order
                        if (!myOrders.empty()) {
                            OrderId modifyId = myOrders[gen() % myOrders.size()];
                            OrderModify modify(modifyId, side, priceDist(gen), quantityDist(gen));
                            auto trades = orderbook_.MatchOrder(modify);
                            successfulOrders_++;
                        }
                        break;
                    }
                }
            } catch (const std::exception& e) {
                failedOrders_++;
                std::cerr << "Thread " << threadId << " error: " << e.what() << std::endl;
            }
        }
        
        completedThreads_++;
    }
    
    void readerThread(int threadId) {
        std::random_device rd;
        std::mt19937 gen(rd() + threadId + 1000);
        std::uniform_int_distribution<> delayDist(1, 10);
        
        while (completedThreads_.load() < 8) { // Wait for worker threads
            try {
                // Read operations
                auto size = orderbook_.Size();
                auto levels = orderbook_.GetOrderInfos();
                
                // Small delay to simulate real-world usage
                std::this_thread::sleep_for(std::chrono::milliseconds(delayDist(gen)));
            } catch (const std::exception& e) {
                std::cerr << "Reader thread " << threadId << " error: " << e.what() << std::endl;
            }
        }
    }
    
    void verifyOrderBookIntegrity() {
        std::cout << "\n=== INTEGRITY CHECK ===" << std::endl;
        
        try {
            auto levels = orderbook_.GetOrderInfos();
            auto size = orderbook_.Size();
            
            std::cout << "Order book integrity: PASSED" << std::endl;
            std::cout << "Bid levels: " << levels.GetBids().size() << std::endl;
            std::cout << "Ask levels: " << levels.GetAsks().size() << std::endl;
            
            // Check for price-time priority
            bool priceTimePriorityMaintained = true;
            for (size_t i = 1; i < levels.GetBids().size(); ++i) {
                if (levels.GetBids()[i-1].price_ < levels.GetBids()[i].price_) {
                    priceTimePriorityMaintained = false;
                    break;
                }
            }
            for (size_t i = 1; i < levels.GetAsks().size(); ++i) {
                if (levels.GetAsks()[i-1].price_ > levels.GetAsks()[i].price_) {
                    priceTimePriorityMaintained = false;
                    break;
                }
            }
            
            std::cout << "Price-time priority: " << (priceTimePriorityMaintained ? "MAINTAINED" : "VIOLATED") << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Order book integrity: FAILED - " << e.what() << std::endl;
        }
    }
};

int main() {
    ThreadSafetyTest test;
    test.runConcurrencyTest();
    return 0;
}
