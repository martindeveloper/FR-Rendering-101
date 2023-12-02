#include "../../Platform/Platform.h"

#include "Renderer.h"

using namespace Graphics::DirectX12;

Renderer::Renderer()
{
    this->Logger = Platform::GetLogger();
}

Renderer::~Renderer()
{
    // No extra need to manually release resources because of ComPtr
}

void Renderer::Initialize(HWND windowHandle, UINT width, UINT height)
{
    this->WindowHandle = windowHandle;

    this->FrameBufferWidth = width;
    this->FrameBufferHeight = height;

    this->CreateDevice();
    this->CreateCommandInterfaces();
    this->CreateSwapChain();
    this->CreateRenderTargetViews();

    this->CreateFrameFence();

    // Triangle entity
    this->Triangle = new TriangleEntity();
    this->Triangle->OnResourceCreate(this->Device);
}

void Renderer::CreateFrameFence()
{
    // Create frame fence
    HRESULT result = this->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->FrameFence));

    if (FAILED(result))
    {
        this->Logger->Fatal("Renderer::CreateFrameFence: Failed to create frame fence");
        return;
    }
}

void Renderer::WaitForGPU()
{
    if (!this->CommandQueue || !this->FrameFence)
    {
        this->Logger->Fatal("Renderer::WaitForGPU: CommandQueue or FrameFence is null");
        return;
    }

    const UINT64 currentFenceValue = this->FrameFenceValues[this->CurrentFrameBufferIndex];
    HRESULT hr = this->CommandQueue->Signal(this->FrameFence.Get(), currentFenceValue);
    if (FAILED(hr))
    {
        this->Logger->Fatal("Renderer::WaitForGPU: Failed to signal command queue");
        return;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3 = nullptr;
    if (FAILED(this->SwapChain.As(&swapChain3)))
    {
        this->Logger->Fatal("Renderer::WaitForGPU: Failed to cast swap chain to IDXGISwapChain3");
        return;
    }

    this->CurrentFrameBufferIndex = swapChain3->GetCurrentBackBufferIndex();
    UINT64 completedFenceValue = this->FrameFence->GetCompletedValue();
    UINT64 frameFenceValue = this->FrameFenceValues[this->CurrentFrameBufferIndex];

    if (completedFenceValue < frameFenceValue)
    {
        this->FrameFence->SetEventOnCompletion(frameFenceValue, this->FrameFenceEvent);
        WaitForSingleObject(this->FrameFenceEvent, INFINITE);
    }

    this->FrameFenceValues[this->CurrentFrameBufferIndex] = currentFenceValue + 1;
}

void Renderer::Render()
{
    if (this->ShouldRender == false)
    {
        return;
    }

    // Ensure that the GPU is done rendering
    this->WaitForGPU();

    // Reset command allocator and command list
    this->CommandAllocator->Reset();
    this->CommandList->Reset(this->CommandAllocator.Get(), nullptr);

    // Transition swap chain buffer to render target
    D3D12_RESOURCE_BARRIER resourceBarrier = {};
    resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

    resourceBarrier.Transition.pResource = this->RenderTargets[this->CurrentFrameBufferIndex].Get();
    resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    this->CommandList->ResourceBarrier(1, &resourceBarrier);

    // Set render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = this->RTVHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += this->CurrentFrameBufferIndex * this->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    this->CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear render target
    if (this->ShouldClear)
    {
        this->CommandList->ClearRenderTargetView(rtvHandle, this->ClearColor, 0, nullptr);
    }

    // Set viewport
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)this->FrameBufferWidth;
    viewport.Height = (FLOAT)this->FrameBufferHeight;
    viewport.MinDepth = D3D12_MIN_DEPTH;
    viewport.MaxDepth = D3D12_MAX_DEPTH;

    this->CommandList->RSSetViewports(1, &viewport);

    // Set scissor rectangle
    D3D12_RECT scissorRectangle = {};
    scissorRectangle.left = 0;
    scissorRectangle.top = 0;
    scissorRectangle.right = this->FrameBufferWidth;
    scissorRectangle.bottom = this->FrameBufferHeight;

    this->CommandList->RSSetScissorRects(1, &scissorRectangle);

    // Triangle entity
    this->Triangle->OnRender(this->FrameCounter, this->CommandList);

    // Transition swap chain buffer to present
    resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    this->CommandList->ResourceBarrier(1, &resourceBarrier);

    // Close command list
    this->CommandList->Close();

    // Execute command list
    ID3D12CommandList *commandLists[] = {this->CommandList.Get()};
    this->CommandQueue->ExecuteCommandLists(1, commandLists);

    // Present
    UINT syncInterval = 1;
    UINT presentFlags = 0;

    this->SwapChain->Present(syncInterval, presentFlags);

    // Ensure that the GPU is done rendering
    this->WaitForGPU();

    // Increment frame counter
    this->FrameCounter++;
}

void Renderer::Resize(UINT width, UINT height)
{
    if (this->SwapChain == nullptr)
    {
        this->Logger->Fatal("Renderer::Resize: Swap chain is null");
        return;
    }

    if (width == this->FrameBufferWidth && height == this->FrameBufferHeight)
    {
        return;
    }

    this->ShouldRender = false;

    this->FrameBufferWidth = width;
    this->FrameBufferHeight = height;

    this->WaitForGPU();

    // Remove old render target views
    this->CleanupRenderTargetViews();

    this->SwapChain->ResizeBuffers(this->BufferCount, width, height, this->FrameBufferFormat, 0);

    // Create new render target views with new buffer size
    this->CreateRenderTargetViews();

    this->WaitForGPU();

    this->ShouldRender = true;
}

void Renderer::CreateDevice()
{
#ifdef BUILD_TYPE_DEBUG
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&this->DebugInterface))))
    {
        this->DebugInterface->EnableDebugLayer();
    }
#endif

    IDXGIFactory7 *dxgiFactory = nullptr;
    HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));

    if (FAILED(result))
    {
        this->Logger->Fatal("DirectX12Renderer: Failed to create DXGI factory");
        return;
    }

    this->DXGIFactory = dxgiFactory;

    this->FindSuitableHardwareAdapter();

    ID3D12Device9 *device = nullptr;
    result = D3D12CreateDevice(this->Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

    if (FAILED(result))
    {
        this->Logger->Fatal("DirectX12Renderer: Failed to create device");
        return;
    }

    this->Device = device;
}

void Renderer::FindSuitableHardwareAdapter()
{
    Microsoft::WRL::ComPtr<IDXGIAdapter1> currentAdapter = nullptr;
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory7 = nullptr;

    if (FAILED(this->DXGIFactory.As(&dxgiFactory7)))
    {
        this->Logger->Fatal("Renderer::FindSuitableHardwareAdapter: Failed to cast DXGI factory to DXGI factory 7");
        return;
    }

    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory7->EnumAdapters1(adapterIndex, &currentAdapter); ++adapterIndex)
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

            this->Logger->Message("Renderer::FindSuitableHardwareAdapter: Found hardware adapter: %ls", description);

            GPUPerformanceClass performanceClass = this->TryToDeterminePerformanceClass(&adapterDescription);

            switch (performanceClass)
            {
            case GPUPerformanceClass::Unknown:
                this->Logger->Message("Renderer::FindSuitableHardwareAdapter: Unknown performance class");
                break;
            case GPUPerformanceClass::Integrated:
                this->Logger->Message("Renderer::FindSuitableHardwareAdapter: Integrated performance class");
                break;
            case GPUPerformanceClass::Dedicated:
                this->Logger->Message("Renderer::FindSuitableHardwareAdapter: Dedicated performance class");
                break;
            }

            break;
        }
    }

    this->Adapter = currentAdapter.Detach();
}

void Renderer::CreateCommandInterfaces()
{
    if (this->Device == nullptr)
    {
        this->Logger->Fatal("Renderer::CreateCommandInterfaces: Device is null");
        return;
    }

    HRESULT result;

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDescription = {};
    queueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Compute and graphics commands

    result = this->Device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&this->CommandQueue));
    Platform::CheckHandle(result, "Failed to create command queue");

    // Create command allocator
    result = this->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->CommandAllocator));
    Platform::CheckHandle(result, "Failed to create command allocator");

    // Create command list
    result = this->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&this->CommandList));
    Platform::CheckHandle(result, "Failed to create command list");

    // Close command list
    this->CommandList->Close();
}

void Renderer::CreateSwapChain()
{
    if (this->DXGIFactory == nullptr || this->WindowHandle == nullptr || this->CommandQueue == nullptr)
    {
        this->Logger->Fatal("Renderer::CreateSwapChain: DXGI factory, window handle or command queue is null");
        return;
    }

    HRESULT result;

    DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};
    swapChainDescription.BufferCount = this->BufferCount;
    swapChainDescription.Width = this->FrameBufferWidth;
    swapChainDescription.Height = this->FrameBufferHeight;
    swapChainDescription.Format = this->FrameBufferFormat;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDescription.SampleDesc.Count = 1; // No anti-aliasing

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain = nullptr;

    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory7 = nullptr;

    result = this->DXGIFactory.As(&dxgiFactory7);
    Platform::CheckHandle(result, "Failed to cast DXGI factory to DXGI factory 7");

    result = dxgiFactory7->CreateSwapChainForHwnd(this->CommandQueue.Get(), this->WindowHandle, &swapChainDescription, nullptr, nullptr, &swapChain);
    Platform::CheckHandle(result, "Failed to create swap chain");

    this->SwapChain = swapChain.Detach();
}

void Renderer::CleanupRenderTargetViews()
{
    for (UINT i = 0; i < this->BufferCount; i++)
    {
        if (this->RenderTargets[i] != nullptr)
        {
            this->RenderTargets[i] = nullptr;
        }
    }

    if (this->RTVHeap != nullptr)
    {
        this->RTVHeap->Release();
        this->RTVHeap = nullptr;
    }
}

void Renderer::CreateRenderTargetViews()
{
    if (this->Device == nullptr || this->SwapChain == nullptr)
    {
        this->Logger->Fatal("Renderer::CreateRenderTargetView: Device or swap chain is null");
        return;
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescription = {};
    rtvHeapDescription.NumDescriptors = this->BufferCount;
    rtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT result = this->Device->CreateDescriptorHeap(&rtvHeapDescription, IID_PPV_ARGS(&this->RTVHeap));

    Platform::CheckHandle(result, "Failed to create descriptor heap");

    UINT rtvDescriptorSize = this->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = this->RTVHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < this->BufferCount; i++)
    {
        result = this->SwapChain->GetBuffer(i, IID_PPV_ARGS(&this->RenderTargets[i]));
        Platform::CheckHandle(result, "Failed to get swap chain buffer");

        this->Device->CreateRenderTargetView(this->RenderTargets[i].Get(), nullptr, rtvHandle);

        rtvHandle.ptr += rtvDescriptorSize;
    }
}

GPUPerformanceClass Renderer::TryToDeterminePerformanceClass(DXGI_ADAPTER_DESC1 *adapterDescription)
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
