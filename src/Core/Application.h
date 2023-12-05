#pragma once

#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include "../Platform/Platform.h"
#include "CoreObject.h"
#include "Window.h"
#include "WindowProperties.h"
#include "../Scene/SceneGraph.h"

namespace Core
{
    /**
     * The application class is the entry point for the application.
     */
    class Application : public CoreObject
    {
    private:
        bool IsInitialized = false;
        Diagnostics::Logger *Logger = nullptr;
        Window *Window = nullptr;
        WindowProperties *WindowProperties = nullptr;
        HWND WindowHandle = nullptr;

    public:
        Application(Platform::Windows::EntrypointPayload payload);
        ~Application();

        /**
         * Initialize the application and create the window.
         */
        bool Initialize();

        /**
         * Set the initial scene graph.
         */
        void SetInitialSceneGraph(Scene::SceneGraph *sceneGraph);

        /**
         * Run the application.
         */
        int Run();

        /**
         * Handle window Win32 messages - static function for Win32.
         */
        static LRESULT CALLBACK WindowProcedureStatic(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam);

    private:
        /**
         * Handle window Win32 messages.
         */
        LRESULT HandleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    };
}

#endif // CORE_APPLICATION_H
