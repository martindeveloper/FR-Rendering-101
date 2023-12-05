#pragma once

#ifndef PLATFORM_WINDOWS_STD_H
#define PLATFORM_WINDOWS_STD_H

#include <numeric>
#include <string>
#include <vector>
#include <memory>
#include <wrl.h>

#include "MemoryAllocator.h"

namespace Platform::Windows
{
    /**
     * @brief Get pointer to Allocator singleton instance
     * @return Pointer to Allocator instance
     */
    static Platform::Windows::MemoryAllocator *GetAllocator()
    {
        static Platform::Windows::MemoryAllocator allocator;
        return &allocator;
    }
}

#endif // PLATFORM_WINDOWS_STD_H
