#pragma once

#ifndef CORE_OBJECT_USE_CUSTOM_ALLOCATOR_MIXIN_H
#define CORE_OBJECT_USE_CUSTOM_ALLOCATOR_MIXIN_H

#include "../Platform/Platform.h"

namespace Core
{
    /**
     * @brief ObjectUseCustomAllocatorMixin is a mixin class that allows objects to use a custom memory allocator
     */
    class ObjectUseCustomAllocatorMixin
    {
    public:
        virtual ~ObjectUseCustomAllocatorMixin() = default;

        /**
         * @brief Overload new and delete operators to use custom memory allocator
         */
        static void *operator new(size_t size)
        {
            return Platform::Memory::Allocate(size);
        }

        /**
         * @brief Overload new[] operator to use custom memory allocator
         */
        static void *operator new[](size_t size)
        {
            return Platform::Memory::Allocate(size);
        }

        /**
         * @brief Overload delete[] operator to use custom memory allocator
         */
        static void operator delete[](void *pointer)
        {
            Platform::Memory::Deallocate(pointer);
        }

        /**
         * @brief Overload new and delete operators to use custom memory allocator
         */
        static void operator delete(void *pointer)
        {
            Platform::Memory::Deallocate(pointer);
        }

        /**
         * @brief Custom malloc function
         */
        static void *malloc(size_t size)
        {
            return Platform::Memory::Allocate(size);
        }

        /**
         * @brief Custom free function
         */
        static void free(void *pointer)
        {
            Platform::Memory::Deallocate(pointer);
        }
    };
}

#endif // CORE_OBJECT_USE_CUSTOM_ALLOCATOR_MIXIN_H
