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

    static void TriggerCrash()
    {
        exit(1);
    };

    static void TriggerBreakpoint()
    {
#if defined(PLATFORM_WINDOWS) && defined(BUILD_TYPE_DEBUG)
        DebugBreak();
#endif
    };

    static void CheckHandle(HRESULT result, const char *message, bool shouldCrash = true)
    {
        if (FAILED(result))
        {
            Platform::GetLogger()->Fatal("Platform::CheckHandle: %s", message);

            Platform::TriggerBreakpoint();

            if (shouldCrash)
            {
                Platform::TriggerCrash();
            }
        }
    };
};
#endif // PLATFORM_H
