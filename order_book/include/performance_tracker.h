#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

// Performance metrics for order book operations
// Tracks timing statistics and call counts for performance analysis
struct PerformanceMetrics {
    std::chrono::nanoseconds totalTime{0};
    std::chrono::nanoseconds minTime{std::chrono::nanoseconds::max()};
    std::chrono::nanoseconds maxTime{0};
    uint64_t callCount{0};
    uint64_t totalOrdersProcessed{0};
    
    // Resets all metrics to initial state
    void reset();
    
    // Calculates and returns the average execution time per operation
    std::chrono::nanoseconds getAverageTime() const;
};

// Simple performance tracker for order book operations
// Provides timing measurement and statistical analysis for various operations
// Can be enabled/disabled for production vs development builds
class PerformanceTracker {
private:
    std::unordered_map<std::string, PerformanceMetrics> metrics_;
    bool enabled_{true};
    
public:
    // Enable or disable performance tracking
    // When disabled, tracking operations become no-ops for minimal overhead
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
    // Start timing an operation
    // Returns a time point that should be passed to recordOperation
    std::chrono::high_resolution_clock::time_point startTimer() const;
    
    // Record timing for an operation
    // Calculates duration from startTime and updates metrics
    // operationName: identifier for the operation being tracked
    // startTime: time point returned by startTimer()
    // ordersProcessed: number of orders processed in this operation (default: 1)
    void recordOperation(const std::string& operationName, 
                        std::chrono::high_resolution_clock::time_point startTime,
                        uint64_t ordersProcessed = 1);
    
    // Get metrics for a specific operation
    // Returns reference to metrics for the specified operation
    // Returns empty metrics if operation not found
    const PerformanceMetrics& getMetrics(const std::string& operationName) const;
    
    // Reset all metrics
    // Clears all recorded performance data
    void reset();
    
    // Print detailed performance report
    // Shows comprehensive statistics for all tracked operations
    void printReport() const;
    
    // Print summary statistics
    // Shows high-level performance overview across all operations
    void printSummary() const;
};
