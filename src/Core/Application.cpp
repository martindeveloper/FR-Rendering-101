#include "../Platform/Platform.h"

#include "Application.h"

using namespace Core;

Application::Application(struct Platform::Windows::EntrypointPayload payload)
{
    this->IsInitialized = false;

    // Initialize logger
    this->Logger = Platform::GetLogger();

    // Initialize window properties
    this->WindowProperties = new Core::WindowProperties();
    this->WindowProperties->Title = STRING_NATIVE("Flying Rat Rendering 101");
    this->WindowProperties->ComClassName = STRING_NATIVE("FlyingRatRendering101");
    this->WindowProperties->WindowShowStyle = payload.nCmdShow;
    this->WindowProperties->Size.Width = 1024;
    this->WindowProperties->Size.Height = 768;
    this->WindowProperties->WindowInstanceHandle = payload.hInstance;

    // Load default, arrow, cursor
    this->WindowProperties->CursorHandle = LoadCursorW(NULL, IDC_ARROW);

    // Create window wrapper
    this->Window = new Core::Window(this->WindowProperties);
}

Application::~Application()
{
    // Unregister window class
    BOOL isClassUnregistered = UnregisterClassW(this->WindowProperties->ComClassName, this->WindowProperties->WindowInstanceHandle);

    // Unload cursor
    BOOL isCursorDestroyed = DestroyCursor(this->WindowProperties->CursorHandle);

    delete this->Window;
    delete this->WindowProperties;
}

LRESULT CALLBACK Application::WindowProcedureStatic(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam)
{
    // Get application instance
    // Keep in mind this is "this" pointer, which has access to all class members
    Application *applicationThis = nullptr;

    // Check if window is being created
    if (messageId == WM_NCCREATE)
    {
        CREATESTRUCT *createOptions = (CREATESTRUCT *)lParam;

        // Extract application instance from window creation data and associate it with window handle
        applicationThis = (Application *)createOptions->lpCreateParams;
        SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)applicationThis);

        applicationThis->Window->OnCreate(windowHandle);
    }
    else
    {
        applicationThis = (Application *)GetWindowLongPtr(windowHandle, GWLP_USERDATA);
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
    switch (messageId)
    {
    case WM_DESTROY:
    {
        this->Window->OnQuit();
        return 1;
    }
    break;

    case WM_PAINT:
    {
        this->Window->OnPaint();
        return 1;
    }
    break;

    case WM_SIZE:
    {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);

        this->Window->OnSizeChange(width, height);

        return 1;
    }

    case WM_SETCURSOR:
    {
        this->Window->OnSetCursor();

        return 1;
    }
    break;
    }

    return DefWindowProc(windowHandle, messageId, wParam, lParam);
}

void Application::SetInitialSceneGraph(Scene::SceneGraph *sceneGraph)
{
    this->Window->SetSceneGraph(sceneGraph);
}

bool Application::Initialize()
{
    if (this->IsInitialized)
    {
        this->Logger->Fatal("Application is already initialized");

        return false;
    }

    // Define window class
    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = WindowProcedureStatic;
    windowClass.hInstance = this->WindowProperties->WindowInstanceHandle;
    windowClass.lpszClassName = this->WindowProperties->ComClassName;

    // Register window class
    RegisterClassW(&windowClass);

    // Create window
    this->WindowHandle = CreateWindowExW(
        0,
        this->WindowProperties->ComClassName,
        this->WindowProperties->Title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, this->WindowProperties->Size.Width, this->WindowProperties->Size.Height,
        NULL,
        NULL,
        this->WindowProperties->WindowInstanceHandle,
        this);

    if (this->WindowHandle == NULL)
    {
        this->Logger->Fatal("Failed to create window");

        return false;
    }

    this->IsInitialized = true;

    return true;
}

int Application::Run()
{
    if (!this->IsInitialized)
    {
        this->Logger->Fatal("Application is not initialized, unable to run");

        return -1;
    }

    // Show window
    ShowWindow(this->WindowHandle, this->WindowProperties->WindowShowStyle);

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
