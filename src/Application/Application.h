#pragma once

#include "../Platform/Windows/Windows.h"
#include "../Platform/Windows/Entrypoint.h"
#include "Window/ApplicationWindow.h"
#include "ApplicationWindowProperties.h"

class Application
{
private:
    ApplicationWindow *Window = nullptr;
    ApplicationWindowProperties *WindowProperties = nullptr;

public:
    Application(EntrypointPayload payload);
    ~Application();

    int Run();

    // Window message handler
    static LRESULT CALLBACK WindowProcedureStatic(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam);

private:
    LRESULT HandleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
