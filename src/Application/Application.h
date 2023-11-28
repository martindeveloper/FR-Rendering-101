#pragma once

#include "../Platform/Windows/Windows.h"
#include "../Platform/Windows/Entrypoint.h"

struct ApplicationWindowProperties
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
};

class Application
{
private:
    ApplicationWindowProperties WindowProperties;

public:
    Application(EntrypointPayload payload);
    ~Application();

    int Run();

    // Window message handler
    static LRESULT CALLBACK WindowProcedureStatic(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam);

private:
    LRESULT HandleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
