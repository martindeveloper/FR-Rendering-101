#include "ApplicationWindow.h"

ApplicationWindow::ApplicationWindow(ApplicationWindowProperties *properties)
    : Properties(properties)
{
}

ApplicationWindow::~ApplicationWindow()
{
}

void ApplicationWindow::OnCreate(HWND windowHandle)
{
    this->Properties->WindowHandle = windowHandle;
}

void ApplicationWindow::OnQuit()
{
    PostQuitMessage(0);
}

void ApplicationWindow::OnPaint()
{
    PAINTSTRUCT paintOptions;

    HDC deviceContextHandle = BeginPaint(this->Properties->WindowHandle, &paintOptions);
    FillRect(deviceContextHandle, &paintOptions.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

    EndPaint(this->Properties->WindowHandle, &paintOptions);
}

void ApplicationWindow::OnSetCursor()
{
    SetCursor(this->Properties->CursorHandle);
}
