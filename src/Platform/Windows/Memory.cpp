#include "Memory.h"

MemoryTracker::MemoryTracker()
{
    this->Logger = Platform::GetLogger();
}

void MemoryTracker::Add(void *ptr, size_t size)
{
    std::lock_guard<std::mutex> lock(InternalLock);
    AllocationsHistory[ptr] = size;
}

void MemoryTracker::Remove(void *ptr)
{
    std::lock_guard<std::mutex> lock(InternalLock);
    AllocationsHistory.erase(ptr);
}

void MemoryTracker::PrintStats()
{
    std::lock_guard<std::mutex> lock(InternalLock);
    size_t totalSize = 0;

    for (const auto &pair : AllocationsHistory)
    {
        totalSize += pair.second;
    }

    this->Logger->Message("Total allocated memory: %zu bytes", totalSize);
}

WindowsAllocator::WindowsAllocator()
{
#if WINDOWS_ALLOCATOR_DEBUG_STATS
    this->Logger = Platform::GetLogger();
    this->Tracker = new MemoryTracker();
#endif
}

WindowsAllocator::~WindowsAllocator()
{
#if WINDOWS_ALLOCATOR_DEBUG_STATS
    this->Logger->Message("WindowsAllocator is being destroyed.");
    this->Tracker->PrintStats();

    delete this->Tracker;
#endif
}

void *WindowsAllocator::Allocate(size_t size)
{
    void *memoryPointer = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#if WINDOWS_ALLOCATOR_DEBUG_VERBOSE
    this->Logger->Message("Allocated %zu bytes at %p", size, memoryPointer);
#endif
#if WINDOWS_ALLOCATOR_DEBUG_STATS
    this->Tracker->Add(memoryPointer, size);
    this->Tracker->PrintStats();
#endif
    return memoryPointer;
}

void WindowsAllocator::Deallocate(void *ptr, size_t size)
{
#if WINDOWS_ALLOCATOR_DEBUG_STATS
    this->Tracker->Remove(ptr);
#endif
    VirtualFree(ptr, size, MEM_RELEASE);
}

MemoryTracker *WindowsAllocator::GetTracker()
{
#if WINDOWS_ALLOCATOR_DEBUG_STATS
    return this->Tracker;
#endif
    return nullptr;
}

WindowsAllocator &g_GetAllocatorInstance()
{
    static WindowsAllocator allocator;
    return allocator;
}

void *operator new(size_t size)
{
    return g_GetAllocatorInstance().Allocate(size);
}

void operator delete(void *ptr) noexcept
{
    g_GetAllocatorInstance().Deallocate(ptr);
}
