#pragma once

#ifndef GRAPHICS_DIRECTX12_RENDERER_H
#define GRAPHICS_DIRECTX12_RENDERER_H

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "../../Platform/Platform.h"
#include "../../Core/BaseObject.h"
#include "../../Scene/Entities/TriangleEntity.h"
#include "../../Scene/SceneGraph.h"
#include "../../Scene/SceneNode.h"
#include "Vertex.h"
#include "FrameMetadata.h"

namespace Graphics::DirectX12
{
    using namespace Microsoft::WRL;

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
     * @brief The DiagnosticModule enum
     */
    enum class DiagnosticModule
    {
        PixOnWindows
    };

    /**
     * @brief The GpuFenceWaitReason enum
     */
    enum class GpuFenceWaitReason
    {
        None,
        RenderBuffersSizeChange
    };

    /**
     * @brief The Renderer::DirectX12::Renderer class
     */
    class Renderer : public Core::BaseObject
    {
    public:
        static const UINT SwapChainBufferCount = 2;

    private:
        Diagnostics::Logger *Logger = nullptr;

        HWND WindowHandle = nullptr;

#ifdef BUILD_TYPE_DEBUG
        std::vector<HMODULE> DiagnosticsModules;
#endif

        // Frame
        UINT FrameBufferWidth = 0;
        UINT FrameBufferHeight = 0;
        UINT CurrentFrameBufferIndex = 0;
        DXGI_FORMAT FrameBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

        UINT64 FrameFenceValues[Renderer::SwapChainBufferCount] = {1, 1};
        HANDLE FrameFenceEvent = nullptr;

        UINT FrameCounter = 0;
        FLOAT FrameClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        bool IsFrameInFlight = false;

        // DX12 interfaces
        ComPtr<IDXGIFactory7> DXGIFactory = nullptr;
        ComPtr<ID3D12Device9> Device = nullptr;
#ifdef BUILD_TYPE_DEBUG
        ComPtr<ID3D12Debug> DebugInterface = nullptr;
        ComPtr<ID3D12DebugDevice> DebugDevice = nullptr;
#endif
        ComPtr<IDXGIAdapter> Adapter = nullptr;
        ComPtr<IDXGISwapChain4> SwapChain = nullptr;
        ComPtr<ID3D12CommandQueue> CommandQueue = nullptr;
        ComPtr<ID3D12CommandAllocator> CommandAllocators[Renderer::SwapChainBufferCount] = {nullptr, nullptr};
        ComPtr<ID3D12DescriptorHeap> RTVHeap = nullptr;
        ComPtr<ID3D12Fence> FrameFence = nullptr;
        ComPtr<ID3D12GraphicsCommandList> CommandLists[Renderer::SwapChainBufferCount] = {nullptr, nullptr};

        ComPtr<ID3D12Resource> RenderTargets[Renderer::SwapChainBufferCount] = {nullptr, nullptr};

        bool ShouldRender = true;
        bool ShouldClear = true;

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
         * @brief Shutdown renderer
         */
        void Shutdown();

        /**
         * @brief Begin a frame
         */
        FrameMetadata *BeginFrame();

        /**
         * @brief End a frame
         */
        void EndFrame(FrameMetadata *frameMetaData);

        /**
         * @brief Resize swap chain
         * @param width
         * @param height
         * @param minimized
         */
        void Resize(UINT width, UINT height, BOOL minimized = FALSE);

        /**
         * @brief Get device
         * @return
         */
        ComPtr<ID3D12Device9> GetDevice() const { return this->Device; };

    private:
        void CreateDevice();
        void FindSuitableHardwareAdapter();
        void CreateCommandInterfaces();
        void CreateSwapChain();
        void CreateRenderTargetHeap();
        void CreateRenderTargetViews();
        void CleanupRenderTargetViews();
        void CreateCommandLists();
        D3D12_RESOURCE_BARRIER CreateTransitionBarrier(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

        DXGI_FORMAT GetFrameBufferFormat() const { return this->FrameBufferFormat; };

        // Frame
        void CreateFrameFence();

        // Utility functions
        void WaitForGPU(GpuFenceWaitReason reason = GpuFenceWaitReason::None) noexcept;
        void WaitBeforeNextFrame() noexcept;

        GPUPerformanceClass TryToDeterminePerformanceClass(DXGI_ADAPTER_DESC1 *adapterDescription);

#ifdef BUILD_TYPE_DEBUG
        void LoadDiagnosticsModule(Graphics::DirectX12::DiagnosticModule module);
        void UnloadDiagnosticsModules();
#endif
    };
}

#endif // GRAPHICS_DIRECTX12_RENDERER_H
