#pragma once

#ifndef PLATFORM_H
#define PLATFORM_H

#include "Generic.h"

#ifdef PLATFORM_WINDOWS
#include "Windows/Windows.h"
#endif

#include "../Diagnostics/Logger.h"

class Platform
{
public:
    static Logger *GetLogger()
    {
        // TODO: Check platform and return appropriate logger
        static Logger instance;
        return &instance;
    }
};

#endif // PLATFORM_H
