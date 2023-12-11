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

    Graphics::DirectX12::ResourcesInitializationMetadata resourcesInitializationMetadata;
    resourcesInitializationMetadata.Device = this->Renderer->GetDevice();
    resourcesInitializationMetadata.BackBufferCount = this->Renderer->SwapChainBufferCount;

    // Initialize scene graph resources
    if (this->SceneGraph != nullptr)
    {
        for (Scene::SceneNode *sceneNode : *this->SceneGraph)
        {
            sceneNode->OnResourceCreate(&resourcesInitializationMetadata);
        }
    }
}

void Window::OnQuit()
{
    // Shutdown scene graph
    if (this->SceneGraph != nullptr)
    {
        for (Scene::SceneNode *sceneNode : *this->SceneGraph)
        {
            sceneNode->OnShutdown();
        }
    }

    // Shutdown renderer
    this->Renderer->Shutdown();

    // Quit
    PostQuitMessage(0);
}

void Window::OnPaint()
{
    if (this->Properties->Size.IsMinimized)
    {
        return;
    }

    Graphics::DirectX12::FrameMetadata *frameMetaData = this->Renderer->BeginFrame();

    if (this->SceneGraph != nullptr)
    {
        for (Scene::SceneNode *sceneNode : *this->SceneGraph)
        {
            sceneNode->OnUpdate(frameMetaData->Frame);
            sceneNode->OnRender(frameMetaData);
        }
    }

    this->Renderer->EndFrame(frameMetaData);
}

void Window::OnSetCursor()
{
    SetCursor(this->Properties->CursorHandle);
}

void Window::OnSizeChange(UINT width, UINT height, BOOL minimized)
{
    this->Properties->Size.Width = width;
    this->Properties->Size.Height = height;
    this->Properties->Size.IsMinimized = minimized;

    if (this->Renderer != nullptr)
    {
        this->Renderer->Resize(width, height, minimized);
    }
}
