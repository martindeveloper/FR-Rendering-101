#pragma once

#ifndef CORE_WINDOW_H
#define CORE_WINDOW_H

#include "../Platform/Platform.h"
#include "WindowProperties.h"

#include "../Scene/SceneGraph.h"
#include "../Graphics/DirectX12/Renderer.h"

namespace Core
{
    class Window
    {
    private:
        WindowProperties *Properties;
        Graphics::DirectX12::Renderer *Renderer;
        Scene::SceneGraph *SceneGraph;

    public:
        Window(WindowProperties *properties);
        ~Window();

        void SetSceneGraph(Scene::SceneGraph *sceneGraph) { this->SceneGraph = sceneGraph; };

        void OnCreate(HWND windowHandle);
        void OnQuit();
        void OnPaint();
        void OnSetCursor();
        void OnSizeChange(UINT width, UINT height);
    };
}

#endif // CORE_WINDOW_H
