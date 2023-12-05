#pragma once

#ifndef PLATFORM_WINDOWS_MEMORY_H
#define PLATFORM_WINDOWS_MEMORY_H

#include <mutex>

#if WINDOWS_ALLOCATOR_DEBUG_STATS == 1
#include "../MemoryStatistics.h"
#endif

/**
 * @brief MemoryAllocator is a custom memory allocator for Windows using Win32 API.
 */
class MemoryAllocator
{
private:
#if WINDOWS_ALLOCATOR_DEBUG_STATS == 1
    MemoryStatistics MemoryStats;
#endif
public:
    ~MemoryAllocator();

    /**
     * @brief Allocate memory.
     * @param size The size of the memory to allocate.
     * @return The pointer to the allocated memory.
     */
    void *Allocate(size_t size);

    /**
     * @brief Deallocate memory.
     * @param memoryPointer The pointer to the memory to deallocate.
     * @param size The size of the memory to deallocate.
     */
    void Deallocate(void *memoryPointer, size_t size = 0);
};

#if ALLOCATOR_GLOBAL_OVERRIDE == 1
void *operator new(size_t size);
void operator delete(void *memoryPointer) noexcept;
#endif

#endif // PLATFORM_WINDOWS_MEMORY_H
