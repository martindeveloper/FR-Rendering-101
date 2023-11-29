#pragma once

#ifndef RENDERER_DIRECTX12_H
#define RENDERER_DIRECTX12_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <d3dcompiler.h>

#include "../../Platform/Platform.h"

enum class GPUPerformanceClass
{
    Unknown,
    Integrated,
    Dedicated
};

class RendererDirectX12
{
private:
    Logger *Logger = nullptr;

    Microsoft::WRL::ComPtr<IDXGIFactory7> DXGIFactory = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Device9> Device = nullptr;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter = nullptr;

public:
    RendererDirectX12();
    ~RendererDirectX12();

    void Initialize(HWND windowHandle);
    void Render();
    void Resize(UINT width, UINT height);

private:
    void CreateDevice();
    void FindSuitableHardwareAdapter();
    GPUPerformanceClass TryToDeterminePerformanceClass(DXGI_ADAPTER_DESC1 *adapterDescription);
    // void CreateCommandQueue();
    // void CreateSwapChain();
    // void CreateRenderTargetView();
    // void CreateCommandAllocator();
    // void CreateCommandList();
};

#endif // RENDERER_DIRECTX12_H
