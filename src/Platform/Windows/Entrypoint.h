#pragma once

#ifndef PLATFORM_WINDOWS_ENTRYPOINT_H
#define PLATFORM_WINDOWS_ENTRYPOINT_H

#include "./OS.h"

namespace Platform::Windows
{
    struct EntrypointPayload
    {
        HINSTANCE hInstance;
        HINSTANCE hPrevInstance;
        LPSTR pCmdLine;
        int nCmdShow;

        // Initializer
        EntrypointPayload(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
        {
            this->hInstance = hInstance;
            this->hPrevInstance = hPrevInstance;
            this->pCmdLine = pCmdLine;
            this->nCmdShow = nCmdShow;
        };
    };
}

#endif // PLATFORM_WINDOWS_ENTRYPOINT_H
