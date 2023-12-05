#pragma once

#ifndef CORE_OBJECT_H
#define CORE_OBJECT_H

#include <iostream>
#include "../Platform/Platform.h"

namespace Core
{
    class BaseObject
    {
    public:
        BaseObject() {}
        virtual ~BaseObject() {}

        static void *operator new(size_t size)
        {
            void *pointer = Platform::Memory::Allocate(size);

            return pointer;
        }

        static void operator delete(void *pointer)
        {
            Platform::Memory::Deallocate(pointer);
        }
    };
}

#endif // CORE_OBJECT_H
