#pragma once

#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include "../Platform/Platform.h"
#include "WindowProperties.h"

#include "../Graphics/DirectX12/Renderer.h"

namespace Core
{
    class Window
    {
    private:
        WindowProperties *Properties;
        Graphics::DirectX12::Renderer *Renderer;

    public:
        Window(WindowProperties *properties);
        ~Window();

        void OnCreate(HWND windowHandle);
        void OnQuit();
        void OnPaint();
        void OnSetCursor();
        void OnSizeChange(UINT width, UINT height);
    };
}

#endif // APPLICATION_WINDOW_H
