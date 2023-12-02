#pragma once

#ifndef RENDERER_DIRECTX12_H
#define RENDERER_DIRECTX12_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>

#include "../../Platform/Platform.h"
#include "Vertex.h"
#include "../../Entities/TriangleEntity.h"

namespace Graphics::DirectX12
{
    /**
     * @brief The GPUPerformanceClass enum
     */
    enum class GPUPerformanceClass
    {
        Unknown,
        Integrated,
        Dedicated
    };

    /**
     * @brief The Renderer::DirectX12::Renderer class
     */
    class Renderer
    {
    private:
        Logger *Logger = nullptr;

        HWND WindowHandle = nullptr;

        DXGI_FORMAT FrameBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT FrameBufferWidth = 0;
        UINT FrameBufferHeight = 0;
        static const UINT BufferCount = 2;
        UINT CurrentFrameBufferIndex = 0;

        Microsoft::WRL::ComPtr<IDXGIFactory> DXGIFactory = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Device> Device = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Debug> DebugInterface = nullptr;
        Microsoft::WRL::ComPtr<IDXGIAdapter> Adapter = nullptr;
        Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain = nullptr;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue = nullptr;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator = nullptr;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList = nullptr;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RTVHeap = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargets[Renderer::BufferCount] = {nullptr, nullptr};

        // Frame fence
        Microsoft::WRL::ComPtr<ID3D12Fence> FrameFence = nullptr;
        UINT64 FrameFenceValues[Renderer::BufferCount] = {1, 1};
        HANDLE FrameFenceEvent = nullptr;

        UINT FrameCounter = 0;
        FLOAT ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

        bool ShouldRender = true;
        bool ShouldClear = true;

        // Triangle entity
        TriangleEntity *Triangle = nullptr;

    public:
        Renderer();
        ~Renderer();

        /**
         * @brief Initialize basic renderer components
         * @param windowHandle
         * @param width
         * @param height
         */
        void Initialize(HWND windowHandle, UINT width, UINT height);

        /**
         * @brief Render one frame
         */
        void Render();

        /**
         * @brief Resize swap chain
         * @param width
         * @param height
         */
        void Resize(UINT width, UINT height);

    private:
        void CreateDevice();
        void FindSuitableHardwareAdapter();
        void CreateCommandInterfaces();
        void CreateSwapChain();
        void CreateRenderTargetViews();
        void CleanupRenderTargetViews();

        // Frame
        void CreateFrameFence();

        // Utility functions
        void WaitForGPU();
        GPUPerformanceClass TryToDeterminePerformanceClass(DXGI_ADAPTER_DESC1 *adapterDescription);
    };
}

#endif // RENDERER_DIRECTX12_H
