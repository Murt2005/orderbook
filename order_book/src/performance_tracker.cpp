#include "performance_tracker.h"
#include <iostream>
#include <iomanip>

void PerformanceMetrics::reset() {
    totalTime = std::chrono::nanoseconds{0};
    minTime = std::chrono::nanoseconds::max();
    maxTime = std::chrono::nanoseconds{0};
    callCount = 0;
    totalOrdersProcessed = 0;
}

std::chrono::nanoseconds PerformanceMetrics::getAverageTime() const {
    if (callCount == 0) return std::chrono::nanoseconds{0};
    return std::chrono::nanoseconds(totalTime.count() / callCount);
}

void PerformanceTracker::setEnabled(bool enabled) { 
    enabled_ = enabled; 
}

bool PerformanceTracker::isEnabled() const { 
    return enabled_; 
}

std::chrono::high_resolution_clock::time_point PerformanceTracker::startTimer() const {
    return std::chrono::high_resolution_clock::now();
}

void PerformanceTracker::recordOperation(const std::string& operationName, 
                    std::chrono::high_resolution_clock::time_point startTime,
                    uint64_t ordersProcessed) {
    if (!enabled_) return;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    
    auto& metric = metrics_[operationName];
    metric.totalTime += duration;
    metric.callCount++;
    metric.totalOrdersProcessed += ordersProcessed;
    
    if (duration < metric.minTime) {
        metric.minTime = duration;
    }
    if (duration > metric.maxTime) {
        metric.maxTime = duration;
    }
}

const PerformanceMetrics& PerformanceTracker::getMetrics(const std::string& operationName) const {
    static const PerformanceMetrics empty;
    auto it = metrics_.find(operationName);
    return it != metrics_.end() ? it->second : empty;
}

void PerformanceTracker::reset() {
    metrics_.clear();
}

void PerformanceTracker::printReport() const {
    if (!enabled_ || metrics_.empty()) return;
    
    std::cout << "\n=== PERFORMANCE REPORT ===" << std::endl;
    std::cout << std::setw(20) << "Operation" 
              << std::setw(12) << "Calls"
              << std::setw(15) << "Total Time (ms)"
              << std::setw(15) << "Avg Time (μs)"
              << std::setw(15) << "Min Time (μs)"
              << std::setw(15) << "Max Time (μs)"
              << std::setw(15) << "Orders/Call"
              << std::endl;
    std::cout << std::string(105, '-') << std::endl;
    
    for (const auto& [operation, metric] : metrics_) {
        auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(metric.totalTime).count();
        auto avgUs = std::chrono::duration_cast<std::chrono::microseconds>(metric.getAverageTime()).count();
        auto minUs = std::chrono::duration_cast<std::chrono::microseconds>(metric.minTime).count();
        auto maxUs = std::chrono::duration_cast<std::chrono::microseconds>(metric.maxTime).count();
        auto ordersPerCall = metric.callCount > 0 ? metric.totalOrdersProcessed / metric.callCount : 0;
        
        std::cout << std::setw(20) << operation
                  << std::setw(12) << metric.callCount
                  << std::setw(15) << totalMs
                  << std::setw(15) << avgUs
                  << std::setw(15) << minUs
                  << std::setw(15) << maxUs
                  << std::setw(15) << ordersPerCall
                  << std::endl;
    }
    std::cout << std::endl;
}

void PerformanceTracker::printSummary() const {
    if (!enabled_ || metrics_.empty()) return;
    
    std::cout << "\n=== PERFORMANCE SUMMARY ===" << std::endl;
    
    uint64_t totalCalls = 0;
    std::chrono::nanoseconds totalTime{0};
    
    for (const auto& [operation, metric] : metrics_) {
        totalCalls += metric.callCount;
        totalTime += metric.totalTime;
    }
    
    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(totalTime).count();
    auto avgMs = totalCalls > 0 ? totalMs / totalCalls : 0;
    
    std::cout << "Total Operations: " << totalCalls << std::endl;
    std::cout << "Total Time: " << totalMs << " ms" << std::endl;
    std::cout << "Average Time per Operation: " << avgMs << " ms" << std::endl;
    std::cout << "Operations per Second: " << (totalMs > 0 ? (totalCalls * 1000) / totalMs : 0) << std::endl;
    std::cout << std::endl;
}
