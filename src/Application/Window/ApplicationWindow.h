#pragma once

#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include "../../Platform/Platform.h"
#include "../ApplicationWindowProperties.h"

#include "../../Renderer/DirectX12/RendererDirectX12.h"

class ApplicationWindow
{
private:
    ApplicationWindowProperties *Properties;
    RendererDirectX12 *Renderer;

public:
    ApplicationWindow(ApplicationWindowProperties *properties);
    ~ApplicationWindow();

    void OnCreate(HWND windowHandle);
    void OnQuit();
    void OnPaint();
    void OnSetCursor();
    void OnSizeChange(UINT width, UINT height);
};

#endif // APPLICATION_WINDOW_H
