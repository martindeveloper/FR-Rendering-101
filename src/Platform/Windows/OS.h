#pragma once

#ifndef PLATFORM_WINDOWS_OS_H
#define PLATFORM_WINDOWS_OS_H

// Windows headers
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#define NOCOMM
#define NOSERVICE
#define NOHELP
#define UNICODE
#include <windows.h>
#undef WIN32_NO_STATUS

#include <tchar.h>

// Create native string LPCWSTR - wchar_t*
#define STRING_NATIVE_WIDE(str) L##str
#define STRING_NATIVE_WIDE_LEN(str) (sizeof(str) / sizeof(str[0]))

// Create native string LPSTR - char*
#define STRING_NATIVE_ANSI(str) str
#define STRING_NATIVE_ANSI_LEN(str) (sizeof(str) / sizeof(str[0]))

// Check if UNICODE is defined
#ifdef UNICODE
#define STRING_NATIVE(str) STRING_NATIVE_WIDE(str)
#define STRING_NATIVE_LEN(str) STRING_NATIVE_WIDE_LEN(str)
#else
#define STRING_NATIVE(str) STRING_NATIVE_ANSI(str)
#define STRING_NATIVE_LEN(str) STRING_NATIVE_ANSI_LEN(str)
#endif

// Print last error
#define WINDOWS_PRINT_LAST_ERROR()                                                         \
    {                                                                                      \
        DWORD error = GetLastError();                                                      \
        LPVOID messageBuffer = NULL;                                                       \
        FormatMessageW(                                                                    \
            FORMAT_MESSAGE_ALLOCATE_BUFFER |                                               \
                FORMAT_MESSAGE_FROM_SYSTEM |                                               \
                FORMAT_MESSAGE_IGNORE_INSERTS,                                             \
            NULL,                                                                          \
            error,                                                                         \
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) & messageBuffer, 0, NULL); \
        OutputDebugStringW((LPCWSTR)messageBuffer);                                        \
        LocalFree(messageBuffer);                                                          \
    }

#endif // PLATFORM_WINDOWS_OS_H
