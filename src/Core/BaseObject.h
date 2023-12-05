#pragma once

#ifndef CORE_OBJECT_H
#define CORE_OBJECT_H

#include <iostream>
#include "../Platform/Platform.h"
#include "ObjectUseCustomAllocatorMixin.h"

namespace Core
{
    /**
     * @brief BaseObject is the base class for all objects in the "engine"
     */
    class BaseObject : public Core::ObjectUseCustomAllocatorMixin
    {
        friend class Core::ObjectUseCustomAllocatorMixin;

    public:
        BaseObject() = default;
        virtual ~BaseObject() = default;
    };
}

#endif // CORE_OBJECT_H
