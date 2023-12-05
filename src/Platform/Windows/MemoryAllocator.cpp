#include "../../Platform/Platform.h"
#include "../../Diagnostics/Logger.h"

#include "MemoryAllocator.h"

Platform::Windows::MemoryAllocator::~MemoryAllocator()
{
#if WINDOWS_ALLOCATOR_DEBUG_STATS == 1
    this->MemoryStats.PrintStats();
#endif
}

void *Platform::Windows::MemoryAllocator::Allocate(size_t size)
{
    void *memoryPointer = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#if WINDOWS_ALLOCATOR_DEBUG_VERBOSE == 1
    Diagnostics::Logger *logger = Platform::GetLogger();
    logger->Message("Allocated memory at %p of size %d", memoryPointer, size);

    if (memoryPointer == nullptr)
    {
        logger->Fatal("Failed to allocate memory");
    }
#endif

#if WINDOWS_ALLOCATOR_DEBUG_STATS == 1
    this->MemoryStats.Allocation(size);
#endif

    return memoryPointer;
}

void Platform::Windows::MemoryAllocator::Deallocate(void *memoryPointer, size_t size)
{
#if WINDOWS_ALLOCATOR_DEBUG_VERBOSE == 1
    Diagnostics::Logger *logger = Platform::GetLogger();
    logger->Message("Deallocate memory at %p of size %d", memoryPointer, size);

    if (memoryPointer == nullptr)
    {
        logger->Fatal("Failed to deallocate memory");
    }
#endif

#if WINDOWS_ALLOCATOR_DEBUG_STATS == 1
    this->MemoryStats.Deallocation(size);
#endif

    VirtualFree(memoryPointer, size, MEM_RELEASE);
}

#if ALLOCATOR_GLOBAL_OVERRIDE == 1
void *operator new(size_t size)
{
    return Platform::Allocate(size);
}

void operator delete(void *ptr) noexcept
{
    Platform::Deallocate(ptr);
}
#endif
