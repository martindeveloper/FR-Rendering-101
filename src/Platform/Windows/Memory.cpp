#include "Memory.h"

WindowsAllocator::~WindowsAllocator()
{
}

void *WindowsAllocator::Allocate(size_t size)
{
    void *memoryPointer = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#if WINDOWS_ALLOCATOR_DEBUG_VERBOSE == 1
    Diagnostics::Logger *logger = Platform::GetLogger();
    logger->Message("Allocated memory at 0x%08X of size %d", memoryPointer, size);

    if (memoryPointer == nullptr)
    {
        logger->Fatal("Failed to allocate memory");
    }
#endif

    return memoryPointer;
}

void WindowsAllocator::Deallocate(void *memoryPointer, size_t size)
{
#if WINDOWS_ALLOCATOR_DEBUG_VERBOSE == 1
    Diagnostics::Logger *logger = Platform::GetLogger();
    logger->Message("Deallocate memory at 0x%08X of size %d", memoryPointer, size);

    if (memoryPointer == nullptr)
    {
        logger->Fatal("Failed to deallocate memory");
    }
#endif

    VirtualFree(memoryPointer, size, MEM_RELEASE);
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
