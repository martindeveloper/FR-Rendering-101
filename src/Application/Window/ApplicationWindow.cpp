#include "ApplicationWindow.h"

ApplicationWindow::ApplicationWindow(ApplicationWindowProperties *properties)
    : Properties(properties)
{
}

ApplicationWindow::~ApplicationWindow()
{
    if (this->Renderer != nullptr)
    {
        delete this->Renderer;
    }
}

void ApplicationWindow::OnCreate(HWND windowHandle)
{
    this->Properties->WindowHandle = windowHandle;

    // Create renderer
    this->Renderer = new RendererDirectX12();
    this->Renderer->Initialize(windowHandle, this->Properties->Size.Width, this->Properties->Size.Height);
}

void ApplicationWindow::OnQuit()
{
    PostQuitMessage(0);
}

void ApplicationWindow::OnPaint()
{
    this->Renderer->Render();
}

void ApplicationWindow::OnSetCursor()
{
    SetCursor(this->Properties->CursorHandle);
}

void ApplicationWindow::OnSizeChange(UINT width, UINT height)
{
    this->Properties->Size.Width = width;
    this->Properties->Size.Height = height;

    if (this->Renderer != nullptr)
    {
        this->Renderer->Resize(width, height);
    }
}
