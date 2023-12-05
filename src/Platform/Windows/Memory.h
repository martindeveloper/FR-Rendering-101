#pragma once

#ifndef PLATFORM_WINDOWS_MEMORY_H
#define PLATFORM_WINDOWS_MEMORY_H

#include <map>
#include <mutex>

#include "OS.h"
#include "../Platform.h"
#include "../../Diagnostics/Logger.h"

class MemoryTracker
{
private:
    std::map<void *, size_t> AllocationsHistory;
    std::mutex InternalLock;
    Diagnostics::Logger *Logger;

public:
    MemoryTracker();
    void Add(void *ptr, size_t size);
    void Remove(void *ptr);
    void PrintStats();
};

class WindowsAllocator
{
private:
    Diagnostics::Logger *Logger;
#if WINDOWS_ALLOCATOR_DEBUG_STATS
    MemoryTracker *Tracker;
#endif

public:
    WindowsAllocator();
    ~WindowsAllocator();
    void *Allocate(size_t size);
    void Deallocate(void *ptr, size_t size = 0);
    MemoryTracker *GetTracker();
};

WindowsAllocator &g_GetAllocatorInstance();
void *operator new(size_t size);
void operator delete(void *ptr) noexcept;

#endif // PLATFORM_WINDOWS_MEMORY_H
