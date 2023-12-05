#pragma once

#ifndef PLATFORM_WINDOWS_MEMORY_H
#define PLATFORM_WINDOWS_MEMORY_H

#include <map>
#include <mutex>

#include "OS.h"
#include "../../Diagnostics/Logger.h"

/**
 * @brief Simple memory tracker class.
 */
class MemoryTracker
{
private:
    std::map<void *, size_t> AllocationsHistory;
    std::mutex InternalLock;

public:
    void Add(void *ptr, size_t size)
    {
        std::lock_guard<std::mutex> lock(InternalLock);
        AllocationsHistory[ptr] = size;
    }

    void Remove(void *ptr)
    {
        std::lock_guard<std::mutex> lock(InternalLock);
        AllocationsHistory.erase(ptr);
    }

    void PrintStats()
    {
        std::lock_guard<std::mutex> lock(InternalLock);
        size_t totalSize = 0;

        for (const std::pair<void *, size_t> &pair : AllocationsHistory)
        {
            totalSize += pair.second;
        }

        Logger::Message("Total allocated memory: %d bytes", totalSize);
    }
};

/**
 * @brief Windows allocator class which allocates memory using VirtualAlloc and other Win32 API functions.
 */
class WindowsAllocator
{
private:
#if WINDOWS_ALLOCATOR_DEBUG_STATS
    MemoryTracker *Tracker;
#endif

public:
#if WINDOWS_ALLOCATOR_DEBUG_STATS
    WindowsAllocator()
    {
        this->Tracker = new MemoryTracker();
    }

    ~WindowsAllocator()
    {
        Logger::Message("WindowsAllocator is being destroyed.");
        this->Tracker->PrintStats();

        delete this->Tracker;
    }
#endif

    void *Allocate(size_t size)
    {
        void *memoryPointer = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

#if WINDOWS_ALLOCATOR_DEBUG_VERBOSE
        Logger::Message("Allocated %d bytes at %p", size, memoryPointer);
#endif
#if WINDOWS_ALLOCATOR_DEBUG_STATS
        this->Tracker->Add(memoryPointer, size);
        this->Tracker->PrintStats();
#endif
        return memoryPointer;
    }

    void Deallocate(void *ptr, size_t size = 0)
    {
#if WINDOWS_ALLOCATOR_DEBUG_STATS
        this->Tracker->Remove(ptr);
#endif
        VirtualFree(ptr, size, MEM_RELEASE);
    }

    MemoryTracker *GetTracker()
    {
#if WINDOWS_ALLOCATOR_DEBUG_STATS
        return this->Tracker.get();
#endif
        return nullptr;
    }
};

/**
 * @brief Global allocator instance.
 * @return WindowsAllocator&
 */
WindowsAllocator &g_GetAllocatorInstance()
{
    static WindowsAllocator allocator;

    return allocator;
}

/**
 * @brief Overloaded new operator.
 * @param size
 * @return void*
 */
void *operator new(size_t size)
{
    return g_GetAllocatorInstance().Allocate(size);
}

/**
 * @brief Overloaded delete operator.
 * @param ptr
 * @return void
 */
void operator delete(void *ptr) noexcept
{
    g_GetAllocatorInstance().Deallocate(ptr);
}

#endif // #ifndef PLATFORM_WINDOWS_MEMORY_H
