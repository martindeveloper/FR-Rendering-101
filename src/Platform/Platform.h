#pragma once

#ifndef PLATFORM_H
#define PLATFORM_H

#include "Generic.h"

#ifdef PLATFORM_WINDOWS
#include "Windows/Windows.h"
#endif

#include "../Diagnostics/Logger.h"

// TODO: Very tied to Windows, should be abstracted in the future to Platform::Windows::*
namespace Platform
{
    /**
     * @brief Get pointer to Logger singleton instance
     * @return Pointer to Logger instance
     */
    static Diagnostics::Logger *GetLogger()
    {
        static Diagnostics::Logger instance;
        return &instance;
    }

    /**
     * @brief Halts the program
     * @param exitCode Exit code
     */
    static void Halt(int exitCode)
    {
        exit(exitCode);
    }

    /**
     * @brief Triggers a breakpoint
     */
    static void TriggerBreakpoint()
    {
#if defined(PLATFORM_WINDOWS) && defined(BUILD_TYPE_DEBUG)
        DebugBreak();
#endif
    }

    /**
     * @brief Triggers a crash and tries to break into debugger
     */
    static void TriggerCrash()
    {
        Platform::TriggerBreakpoint();

        Platform::Halt(1);
    }

    /**
     * @brief Checks HRESULT and crashes if failed
     * @param result HRESULT
     * @param message Message to log
     * @param shouldCrash Should crash if failed
     */
    static void CheckHandle(HRESULT result, const char *message, bool shouldCrash = true)
    {
        if (FAILED(result))
        {
            Platform::GetLogger()->Fatal("Platform::CheckHandle: %s", message);

            if (shouldCrash)
            {
                Platform::TriggerCrash();
            }
        }
    }

    /**
     * @brief Checks HRESULT and crashes if failed
     * @param result HRESULT
     * @param message Message to log
     * @param shouldCrash Should crash if failed
     */
    static bool FileExist(const wchar_t *path)
    {
        DWORD fileAttributes = GetFileAttributesW(path);

        return (fileAttributes == INVALID_FILE_ATTRIBUTES);
    }

    /**
     * @brief Load file into buffer
     * @param path Path to file
     * @param buffer Pointer to buffer
     * @param bufferSize Pointer to buffer size
     * @return True if succeeded, false otherwise
     */
    static bool LoadFileIntoBuffer(const wchar_t *path, char **buffer, size_t *bufferSize)
    {
        // Open file
        FILE *file = nullptr;
        errno_t fileOpenResult = _wfopen_s(&file, path, L"rb");

        if (fileOpenResult != 0)
        {
            Platform::GetLogger()->Fatal("Platform::LoadFileIntoBuffer: Failed to open file %s", path);
            Platform::TriggerCrash();

            return false;
        }

        // Get file size
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate buffer
        *buffer = new char[fileSize];

        // Read file into buffer
        size_t bytesRead = fread(*buffer, 1, fileSize, file);

        if (bytesRead != fileSize)
        {
            Platform::GetLogger()->Fatal("Platform::LoadFileIntoBuffer: Failed to read file %s", path);
            Platform::TriggerCrash();

            return false;
        }

        // Close file
        fclose(file);

        // Set buffer size
        *bufferSize = fileSize;

        return true;
    }
}

#endif // PLATFORM_H
