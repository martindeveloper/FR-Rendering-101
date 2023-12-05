#pragma once

#ifndef CORE_OBJECT_H
#define CORE_OBJECT_H

#include <iostream>
#include "../Platform/Platform.h"

namespace Core
{
    class CoreObject
    {
    public:
        CoreObject() {}
        virtual ~CoreObject() {}

        static void *operator new(size_t size)
        {
            void *pointer = Platform::Allocate(size);

            return pointer;
        }

        static void operator delete(void *pointer)
        {
            Platform::Deallocate(pointer);
        }
    };
}

#endif // CORE_OBJECT_H
