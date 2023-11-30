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
 * @brief The RendererDirectX12 class
 */
class RendererDirectX12
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
    Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargets[RendererDirectX12::BufferCount] = {nullptr, nullptr};

    // Frame fence
    Microsoft::WRL::ComPtr<ID3D12Fence> FrameFence = nullptr;
    UINT64 FrameFenceValues[RendererDirectX12::BufferCount] = {1, 1};
    HANDLE FrameFenceEvent = nullptr;

    UINT FrameCounter = 0;

    bool ShouldRender = true;

    // Triangle entity
    TriangleEntity *Triangle = nullptr;

public:
    RendererDirectX12();
    ~RendererDirectX12();

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

    // Frame fence
    void CreateFrameFence();

    // Utility functions
    void WaitForGPU();
    GPUPerformanceClass TryToDeterminePerformanceClass(DXGI_ADAPTER_DESC1 *adapterDescription);
    inline void CheckHandle(HRESULT result, const char *message, bool shouldCrash = true);
};

#endif // RENDERER_DIRECTX12_H
