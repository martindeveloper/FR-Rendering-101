#pragma once

#ifndef DIAGNOSTICS_LOGGER_H
#define DIAGNOSTICS_LOGGER_H

#include <iostream>
#include "../Platform/Generic.h"

namespace Diagnostics
{
    class Logger
    {
    public:
        void Message(const char *message, ...)
        {
            va_list args;
            va_start(args, message);

            char buffer[1024];
            vsprintf_s(buffer, message, args);

            va_end(args);

            this->Log(buffer);
        }

        void Fatal(const char *message, ...)
        {
            va_list args;
            va_start(args, message);

            char buffer[1024];
            vsprintf_s(buffer, message, args);

            va_end(args);

            this->Log(buffer);

#ifdef PLATFORM_WINDOWS
            WINDOWS_PRINT_LAST_ERROR();
#endif
        }

        void Log(const char *message)
        {
#ifdef PLATFORM_WINDOWS
            OutputDebugStringA(message);
            OutputDebugStringA("\n");
#else
            std::cout << message << std::endl;
#endif
        }
    };
}

#endif // DIAGNOSTICS_LOGGER_H
