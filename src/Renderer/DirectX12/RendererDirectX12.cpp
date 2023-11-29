#include "../../Platform/Platform.h"

#include "RendererDirectX12.h"
#include <iostream>

RendererDirectX12::RendererDirectX12()
{
    this->Logger = Platform::GetLogger();
}

RendererDirectX12::~RendererDirectX12()
{
    if (this->DXGIFactory != nullptr)
    {
        this->DXGIFactory->Release();
    }

    if (this->Device != nullptr)
    {
        this->Device->Release();
    }

    this->DXGIFactory = nullptr;
    this->Device = nullptr;
}

void RendererDirectX12::Initialize(HWND windowHandle)
{
    this->CreateDevice();
    // this->CreateCommandQueue();
    // this->CreateSwapChain();
    // this->CreateRenderTargetView();
    // this->CreateCommandAllocator();
    // this->CreateCommandList();
}

void RendererDirectX12::Render()
{
}

void RendererDirectX12::Resize(UINT width, UINT height)
{
}

void RendererDirectX12::CreateDevice()
{
    IDXGIFactory7 *dxgiFactory = nullptr;
    HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12: Failed to create DXGI factory");
        return;
    }

    this->DXGIFactory = dxgiFactory;

    this->FindSuitableHardwareAdapter();

    ID3D12Device9 *device = nullptr;
    result = D3D12CreateDevice(this->Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12: Failed to create device");
        return;
    }

    this->Device = device;
}

void RendererDirectX12::FindSuitableHardwareAdapter()
{
    Microsoft::WRL::ComPtr<IDXGIAdapter1> currentAdapter = nullptr;

    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != this->DXGIFactory->EnumAdapters1(adapterIndex, &currentAdapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 adapterDescription;
        currentAdapter->GetDesc1(&adapterDescription);

        if (adapterDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        // Check if adapter supports Direct3D 12 but don't create the actual device yet
        if (SUCCEEDED(D3D12CreateDevice(currentAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device9), nullptr)))
        {
            wchar_t *description = adapterDescription.Description;

            this->Logger->Message("RendererDirectX12: Found hardware adapter: %ls", description);

            GPUPerformanceClass performanceClass = this->TryToDeterminePerformanceClass(&adapterDescription);

            switch (performanceClass)
            {
            case GPUPerformanceClass::Unknown:
                this->Logger->Message("RendererDirectX12: Unknown performance class");
                break;
            case GPUPerformanceClass::Integrated:
                this->Logger->Message("RendererDirectX12: Integrated performance class");
                break;
            case GPUPerformanceClass::Dedicated:
                this->Logger->Message("RendererDirectX12: Dedicated performance class");
                break;
            }

            break;
        }
    }

    this->Adapter = currentAdapter.Detach();
}

GPUPerformanceClass RendererDirectX12::TryToDeterminePerformanceClass(DXGI_ADAPTER_DESC1 *adapterDescription)
{
    switch (adapterDescription->VendorId)
    {
    case 0x10DE: // NVIDIA
        return GPUPerformanceClass::Dedicated;
    case 0x1002: // AMD
        return GPUPerformanceClass::Dedicated;
    case 0x8086: // Intel, may not be accurate because of their new Xe GPUs
        return GPUPerformanceClass::Integrated;
    default:
        return GPUPerformanceClass::Unknown;
    }
}
