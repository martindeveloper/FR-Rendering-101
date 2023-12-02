#pragma once

#include "../Platform/Platform.h"
#include "Window/ApplicationWindow.h"
#include "ApplicationWindowProperties.h"

class Application
{
private:
    Diagnostics::Logger *Logger = nullptr;
    ApplicationWindow *Window = nullptr;
    ApplicationWindowProperties *WindowProperties = nullptr;

public:
    Application(Platform::Windows::EntrypointPayload payload);
    ~Application();

    int Run();

    // Window message handler
    static LRESULT CALLBACK WindowProcedureStatic(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam);

private:
    LRESULT HandleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
