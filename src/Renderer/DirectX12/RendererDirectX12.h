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

    // Triangle specific
    Microsoft::WRL::ComPtr<ID3D12RootSignature> TriangleRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> TrianglePipelineState = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> TriangleVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> TrianglePixelShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> TriangleVertexBuffer = nullptr;

    const Vertex TriangleVertices[3] = {
        {DirectX::XMFLOAT3(0.0f, 0.25f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
        {DirectX::XMFLOAT3(0.25f, -0.25f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
        {DirectX::XMFLOAT3(-0.25f, -0.25f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}};

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

    // Triangle specific
    void TriangleCreateRootSignature();
    void TriangleCompileShaders();
    void TriangleCreatePipelineState();
    void TriangleCreateVertexBuffer();

    // Utility functions
    void WaitForGPU();
    GPUPerformanceClass TryToDeterminePerformanceClass(DXGI_ADAPTER_DESC1 *adapterDescription);
    inline void CheckHandle(HRESULT result, const char *message, bool shouldCrash = true);
};

#endif // RENDERER_DIRECTX12_H
