#include "Windows/OS.h"
#include "../Diagnostics/Logger.h"
#include "../Platform/Platform.h"

#include "MemoryStatistics.h"

void MemoryStatistics::Allocation(size_t size)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->TotalAllocated += size;
    this->CurrentAllocated += size;
}

void MemoryStatistics::Deallocation(size_t size)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->TotalDeallocated += size;
    this->CurrentDeallocated += size;
}

void MemoryStatistics::PrintStats()
{
    std::lock_guard<std::mutex> lock(mutex);

    Diagnostics::Logger *logger = Platform::GetLogger();

    logger->Message("Total allocated: %d", this->TotalAllocated);
    logger->Message("Total deallocated: %d", this->TotalDeallocated);
    logger->Message("Current allocated: %d", this->CurrentAllocated);
    logger->Message("Current deallocated: %d", this->CurrentDeallocated);
}
