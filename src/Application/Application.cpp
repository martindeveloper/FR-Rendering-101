#include "Application.h"

Application::Application(struct EntrypointPayload payload)
{
    this->WindowProperties = {};
    this->WindowProperties.Title = STRING_NATIVE("Flying Rat Rendering 101");
    this->WindowProperties.ComClassName = STRING_NATIVE("FlyingRatRendering101");
    this->WindowProperties.WindowShowStyle = payload.nCmdShow;
    this->WindowProperties.Size.Width = 1024;
    this->WindowProperties.Size.Height = 768;
    this->WindowProperties.WindowInstanceHandle = payload.hInstance;
}

Application::~Application()
{
}

LRESULT CALLBACK Application::WindowProcedureStatic(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam)
{
    // Get application instance
    // Keep in mind this is "this" pointer, which has access to all class members
    Application* applicationThis = nullptr;
    
    // Check if window is being created
    if (messageId == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;

        // Extract application instance from window creation data and associate it with window handle
        applicationThis = (Application*)pCreate->lpCreateParams;
        SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)applicationThis);

        applicationThis->WindowProperties.WindowHandle = windowHandle;
	}
    else
    {
		applicationThis = (Application*)GetWindowLongPtr(windowHandle, GWLP_USERDATA);
	}

    if (applicationThis)
    {
		return applicationThis->HandleWindowMessage(windowHandle, messageId, wParam, lParam);
	}

    // Should mostly happen only for WM_GETMINMAXINFO and WM_NCCREATE
	return DefWindowProc(windowHandle, messageId, wParam, lParam);
}

LRESULT Application::HandleWindowMessage(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam)
{
    // Handle window messages
    switch (messageId)
    {
    case WM_DESTROY:
        PostQuitMessage(0);

        return 0;

    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(windowHandle, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(windowHandle, &ps);

        return 0;
    }

    return DefWindowProc(windowHandle, messageId, wParam, lParam);
}

int Application::Run()
{
    // Define window class
    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = WindowProcedureStatic;
    windowClass.hInstance = this->WindowProperties.WindowInstanceHandle;
    windowClass.lpszClassName = this->WindowProperties.ComClassName;

    // Register window class
    RegisterClassW(&windowClass);

    // Create window
    HWND windowHandle = CreateWindowExW(
        0,
        this->WindowProperties.ComClassName,
        this->WindowProperties.Title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, this->WindowProperties.Size.Width, this->WindowProperties.Size.Height,
        NULL,
        NULL,
        this->WindowProperties.WindowInstanceHandle,
        this);

    if (windowHandle == NULL)
    {
        ERROR_FATAL("Failed to create window!");

        return 0;
    }

    // Show window
    ShowWindow(windowHandle, this->WindowProperties.WindowShowStyle);

    // Message loop
    MSG windowMessage = {};

    // Pump messages until WM_QUIT
    while (GetMessage(&windowMessage, NULL, 0, 0))
    {
        TranslateMessage(&windowMessage);
        DispatchMessage(&windowMessage);
    }

    return 0;
}