#pragma once

#ifndef CORE_WINDOW_PROPERTIES_H
#define CORE_WINDOW_PROPERTIES_H

#include "../Platform/Windows/Windows.h"

namespace Core
{
    struct WindowProperties
    {
        LPCWSTR Title;
        LPCWSTR ComClassName;
        int WindowShowStyle;

        struct Size
        {
            int Width;
            int Height;
        } Size;

        // Win32 handles
        HINSTANCE WindowInstanceHandle;
        HWND WindowHandle;
        HCURSOR CursorHandle;
    };
}

#endif // CORE_WINDOW_PROPERTIES_H
