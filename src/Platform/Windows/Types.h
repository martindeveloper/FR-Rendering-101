#pragma once

#ifndef PLATFORM_WINDOWS_TYPES_H
#define PLATFORM_WINDOWS_TYPES_H

#include "./OS.h"

namespace Platform::Windows
{
    typedef HWND WindowHandle;
    typedef HINSTANCE InstanceHandle;
    typedef HMODULE ModuleHandle;
    typedef LPCWSTR NativeString;
    typedef LPSTR NativeStringAnsi;
}

#endif // PLATFORM_WINDOWS_TYPES_H
