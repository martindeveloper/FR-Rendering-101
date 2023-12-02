#include "Window.h"

using namespace Core;

Window::Window(WindowProperties *properties)
    : Properties(properties)
{
}

Window::~Window()
{
    if (this->Renderer != nullptr)
    {
        delete this->Renderer;
    }
}

void Window::OnCreate(HWND windowHandle)
{
    this->Properties->WindowHandle = windowHandle;

    // Create renderer
    this->Renderer = new Graphics::DirectX12::Renderer();
    this->Renderer->Initialize(windowHandle, this->Properties->Size.Width, this->Properties->Size.Height);
}

void Window::OnQuit()
{
    PostQuitMessage(0);
}

void Window::OnPaint()
{
    this->Renderer->Render();
}

void Window::OnSetCursor()
{
    SetCursor(this->Properties->CursorHandle);
}

void Window::OnSizeChange(UINT width, UINT height)
{
    this->Properties->Size.Width = width;
    this->Properties->Size.Height = height;

    if (this->Renderer != nullptr)
    {
        this->Renderer->Resize(width, height);
    }
}
