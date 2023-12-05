#pragma once

#ifndef PLATFORM_WINDOWS_MEMORY_H
#define PLATFORM_WINDOWS_MEMORY_H

#include <mutex>

#ifdef WINDOWS_ALLOCATOR_DEBUG_STATS
#include <iostream>

class MemoryTracker
{
};
#endif

#include "OS.h"
#include "../Platform.h"
#include "../../Diagnostics/Logger.h"

class WindowsAllocator
{
private:
#ifdef WINDOWS_ALLOCATOR_DEBUG_STATS
    MemoryTracker tracker;
#endif
public:
    ~WindowsAllocator();
    void *Allocate(size_t size);
    void Deallocate(void *memoryPointer, size_t size = 0);
};

WindowsAllocator &g_GetAllocatorInstance();
void *operator new(size_t size);
void operator delete(void *memoryPointer) noexcept;

#endif // PLATFORM_WINDOWS_MEMORY_H
