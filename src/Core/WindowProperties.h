#pragma once

#ifndef CORE_WINDOW_PROPERTIES_H
#define CORE_WINDOW_PROPERTIES_H

#include "../Platform/Windows/Windows.h"
#include "../Scene/SceneGraph.h"

namespace Core
{
    struct WindowProperties
    {
        LPCWSTR Title;
        LPCWSTR ComClassName;
        int WindowShowStyle;

        struct Size
        {
            int Width;
            int Height;
        } Size;

        Scene::SceneGraph *InitialSceneGraph;

        // Win32 handles
        HINSTANCE WindowInstanceHandle;
        HWND WindowHandle;
        HCURSOR CursorHandle;
    };
}

#endif // CORE_WINDOW_PROPERTIES_H
