#pragma once

#ifndef PLATFORM_WINDOWS_MEMORY_STATISTICS_H
#define PLATFORM_WINDOWS_MEMORY_STATISTICS_H

#include <mutex>

/**
 * @brief MemoryStatistics is a class that keeps track of memory allocation and deallocation.
 */
class MemoryStatistics
{
private:
    std::mutex mutex;
    size_t TotalAllocated = 0;
    size_t TotalDeallocated = 0;
    size_t CurrentAllocated = 0;
    size_t CurrentDeallocated = 0;

public:
    void Allocation(size_t size);
    void Deallocation(size_t size);
    void PrintStats();
};

#endif // PLATFORM_WINDOWS_MEMORY_STATISTICS_H
