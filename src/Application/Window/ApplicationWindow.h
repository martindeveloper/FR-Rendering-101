#pragma once

#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include "../../Platform/Platform.h"
#include "../ApplicationWindowProperties.h"

#include "../../Graphics/DirectX12/Renderer.h"

class ApplicationWindow
{
private:
    ApplicationWindowProperties *Properties;
    Graphics::DirectX12::Renderer *Renderer;

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
