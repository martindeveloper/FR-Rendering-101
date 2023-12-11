#include "../../Platform/Platform.h"

#include "Renderer.h"

using namespace Microsoft::WRL;
using namespace Graphics::DirectX12;

Renderer::Renderer()
{
    this->Logger = Platform::GetLogger();
}

Renderer::~Renderer()
{
    this->Logger->Message("Renderer::~Renderer: Renderer destroyed");
}

void Renderer::Shutdown()
{
    this->Logger->Message("Renderer::Shutdown: Renderer shutdown");

    // Wait for GPU to finish
    this->ShouldRender = false;
    this->WaitForGPU();

    // Release resources
    this->SwapChain.Reset();
    this->Device.Reset();
    this->Adapter.Reset();
    this->DXGIFactory.Reset();

    // Cleanup
    this->CleanupRenderTargetViews();

    CloseHandle(this->FrameFenceEvent);

    this->UnloadDiagnosticsModules();

    this->FrameFence.Reset();

    for (UINT i = 0; i < this->SwapChainBufferCount; i++)
    {
        this->CommandAllocators[i].Reset();
    }

    this->RTVHeap.Reset();
    this->CommandQueue.Reset();

#ifdef BUILD_TYPE_DEBUG
    this->DebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL);

    this->DebugInterface.Reset();
    this->DebugDevice.Reset();
#endif
}

void Renderer::Initialize(HWND windowHandle, UINT width, UINT height)
{
    this->WindowHandle = windowHandle;

    this->FrameBufferWidth = width;
    this->FrameBufferHeight = height;

#ifdef PIX_WINDOWS_ENABLED
    this->LoadDiagnosticsModule(DiagnosticModule::PixOnWindows);
#endif

    this->CreateDevice();
    this->CreateCommandInterfaces();
    this->CreateSwapChain();

    this->CreateRenderTargetHeap();
    this->CreateRenderTargetViews();

    this->CreateFrameFence();
}

void Renderer::LoadDiagnosticsModule(Graphics::DirectX12::DiagnosticModule module)
{
    HMODULE diagnosticsModule = nullptr;
    const char *diagnosticsModulePath = nullptr;

    switch (module)
    {
#ifdef PIX_WINDOWS_ENABLED
    case Graphics::DirectX12::DiagnosticModule::PixOnWindows:
        diagnosticsModulePath = PIX_WINDOWS_CAPTURER_DLL;
        break;
#endif
    }

    diagnosticsModule = LoadLibraryA(diagnosticsModulePath);

    if (diagnosticsModule == nullptr)
    {
        this->Logger->Fatal("Renderer::LoadDiagnosticsModule: Failed to load diagnostics module %s", diagnosticsModulePath);
        return;
    }

    this->Logger->Message("Renderer::LoadDiagnosticsModule: Loaded diagnostics module %s", diagnosticsModulePath);
    this->DiagnosticsModules.push_back(diagnosticsModule);
}

void Renderer::UnloadDiagnosticsModules()
{
    for (HMODULE diagnosticsModule : this->DiagnosticsModules)
    {
        FreeLibrary(diagnosticsModule);
    }
}

void Renderer::CreateFrameFence()
{
    // Create frame fence
    HRESULT result = this->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->FrameFence));

    Platform::CheckHandle(result, "Failed to create frame fence", true);

    // Create frame fence event
    this->FrameFenceEvent = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
}

void Renderer::WaitBeforeNextFrame() noexcept
{
    // As we are using double buffering we must make sure the previous frame is done as we will reuse the RTVs and other
    // This is called after Present() and before BeginFrame()
    // So we started frame #2 and we are waiting for frame #1 to finish
    // We cant start frame #3 until frame #1 is done, due shared resources

    const UINT64 currentFenceValue = this->FrameFenceValues[this->CurrentFrameBufferIndex];

    this->CommandQueue->Signal(this->FrameFence.Get(), currentFenceValue);
    this->CurrentFrameBufferIndex = this->SwapChain->GetCurrentBackBufferIndex();

    if (this->FrameFence->GetCompletedValue() < this->FrameFenceValues[this->CurrentFrameBufferIndex])
    {
        this->FrameFence->SetEventOnCompletion(this->FrameFenceValues[this->CurrentFrameBufferIndex], this->FrameFenceEvent);
        WaitForSingleObjectEx(this->FrameFenceEvent, INFINITE, false);
    }

    this->FrameFenceValues[this->CurrentFrameBufferIndex] = currentFenceValue + 1;
}

void Renderer::WaitForGPU(GpuFenceWaitReason reason) noexcept
{
    // Wait for current frame to finish
    // For example we started frame #2 which waits for #1 to finish
    // Means frame #1 is done at this moment, but frame #2 is not
    // This function waits for frame #2 to finish
    // After this no GPU work should be in flight

    if (this->CommandQueue == nullptr || this->FrameFence == nullptr || this->FrameFenceEvent == nullptr || this->FrameFenceEvent == INVALID_HANDLE_VALUE)
    {
        this->Logger->Fatal("Renderer::WaitForGPU: Command queue, frame fence or frame fence event is null");
        return;
    }

    HRESULT result;
    UINT64 currentFenceValue = this->FrameFenceValues[this->CurrentFrameBufferIndex];

    result = this->CommandQueue->Signal(this->FrameFence.Get(), currentFenceValue);
    Platform::CheckHandle(result, "Failed to signal frame fence");

    result = this->FrameFence->SetEventOnCompletion(currentFenceValue, this->FrameFenceEvent);
    Platform::CheckHandle(result, "Failed to set event on frame fence");

    WaitForSingleObjectEx(this->FrameFenceEvent, INFINITE, false);

    this->FrameFenceValues[this->CurrentFrameBufferIndex]++;
}

FrameMetadata *Renderer::BeginFrame()
{
    if (this->ShouldRender == false)
    {
        this->Logger->Fatal("Renderer::BeginFrame: ShouldRender is false");
        return nullptr;
    }

    if (this->IsFrameInFlight)
    {
        this->Logger->Fatal("Renderer::BeginFrame: Frame is already in flight");
        return nullptr;
    }

    this->IsFrameInFlight = true;

    // Reset command allocator and command list
    ComPtr<ID3D12CommandAllocator> commandAllocator = this->CommandAllocators[this->CurrentFrameBufferIndex];
    commandAllocator->Reset();

    // Create new command list for frame
    ComPtr<ID3D12GraphicsCommandList> commandList = this->CreateCommandList();

    // Reset command list and start recording
    commandList->Reset(commandAllocator.Get(), nullptr);

    // Set viewport
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)this->FrameBufferWidth;
    viewport.Height = (FLOAT)this->FrameBufferHeight;
    viewport.MinDepth = D3D12_MIN_DEPTH;
    viewport.MaxDepth = D3D12_MAX_DEPTH;

    commandList->RSSetViewports(1, &viewport);

    // Set scissor rectangle
    D3D12_RECT scissorRectangle = {};
    scissorRectangle.left = 0;
    scissorRectangle.top = 0;
    scissorRectangle.right = this->FrameBufferWidth;
    scissorRectangle.bottom = this->FrameBufferHeight;

    commandList->RSSetScissorRects(1, &scissorRectangle);

    D3D12_RESOURCE_BARRIER barrier = this->CreateTransitionBarrier(this->RenderTargets[this->CurrentFrameBufferIndex],
                                                                   D3D12_RESOURCE_STATE_PRESENT,
                                                                   D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    // Set render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = this->RTVHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += this->CurrentFrameBufferIndex * this->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear render target
    if (this->ShouldClear)
    {
        commandList->ClearRenderTargetView(rtvHandle, this->FrameClearColor, 0, nullptr);
    }

    FrameMetadata *currentFrameMetaData = new FrameMetadata();

    // Set command list and frame counter
    currentFrameMetaData->CommandList = commandList;
    currentFrameMetaData->Frame = this->FrameCounter;
    currentFrameMetaData->CurrentBackBufferIndex = this->CurrentFrameBufferIndex;

    return currentFrameMetaData;
}

void Renderer::EndFrame(FrameMetadata *frameMetaData)
{
    ComPtr<ID3D12GraphicsCommandList> commandList = frameMetaData->CommandList;

    D3D12_RESOURCE_BARRIER barrier = this->CreateTransitionBarrier(this->RenderTargets[this->CurrentFrameBufferIndex],
                                                                   D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                   D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);

    // Close command list
    commandList->Close();

    // Execute command list
    ID3D12CommandList *commandLists[] = {commandList.Get()};
    this->CommandQueue->ExecuteCommandLists(1, commandLists);

    // Present
    UINT syncInterval = 1;
    UINT presentFlags = 0;

    this->SwapChain->Present(syncInterval, presentFlags);

    // Ensure that the GPU is done rendering
    this->WaitBeforeNextFrame();

    // Increment frame counter
    this->FrameCounter++;

    this->IsFrameInFlight = false;

    delete frameMetaData;
}

void Renderer::Resize(UINT width, UINT height, BOOL minimized)
{
    if (this->SwapChain == nullptr)
    {
        this->Logger->Fatal("Renderer::Resize: Swap chain is null");
        return;
    }

    // Don't render if window is minimized
    this->ShouldRender = !minimized;

    if (minimized)
    {
        this->Logger->Message("Renderer::Resize: Window is minimized");
        return;
    }

    if (width == this->FrameBufferWidth && height == this->FrameBufferHeight)
    {
        return;
    }

    this->Logger->Message("Renderer::Resize: Resizing to %ux%u", width, height);

    this->ShouldRender = false;

    // We must wait for both swap chain buffers to be released before resizing
    this->WaitForGPU(GpuFenceWaitReason::RenderBuffersSizeChange);

    this->FrameBufferWidth = width;
    this->FrameBufferHeight = height;

    // Remove old render target views
    this->CleanupRenderTargetViews();

    // TODO: The PIX handle warning appears here
    HRESULT buffersResizeResult = this->SwapChain->ResizeBuffers(this->SwapChainBufferCount, width, height, this->FrameBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    Platform::CheckHandle(buffersResizeResult, "Failed to resize swap chain buffers", true);

    // Create new render target views with new buffer size
    this->CreateRenderTargetViews();

    this->CurrentFrameBufferIndex = this->SwapChain->GetCurrentBackBufferIndex();

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

    HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&this->DXGIFactory));
    Platform::CheckHandle(result, "Failed to create DXGI factory", true);

    this->FindSuitableHardwareAdapter();

    result = D3D12CreateDevice(this->Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&this->Device));
    Platform::CheckHandle(result, "Failed to create device", true);

#ifdef BUILD_TYPE_DEBUG
    result = this->Device.As(&this->DebugDevice);
    Platform::CheckHandle(result, "Failed to cast device to debug device", true);
#endif
}

void Renderer::FindSuitableHardwareAdapter()
{
    ComPtr<IDXGIAdapter1> currentAdapter = nullptr;
    ComPtr<IDXGIFactory7> dxgiFactory7 = nullptr;
    HRESULT castResult = this->DXGIFactory.As(&dxgiFactory7);

    Platform::CheckHandle(castResult, "Failed to cast DXGI factory to DXGI factory 7", true);

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

    // One queue is fine for two swap chain buffers
    result = this->Device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&this->CommandQueue));
    Platform::CheckHandle(result, "Failed to create command queue");

#ifdef BUILD_TYPE_DEBUG
    this->CommandQueue->SetName(L"Command Queue Primary");
#endif

    for (UINT i = 0; i < this->SwapChainBufferCount; i++)
    {

        result = this->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->CommandAllocators[i]));
        Platform::CheckHandle(result, "Failed to create command allocator");

#ifdef BUILD_TYPE_DEBUG
        wchar_t name[25] = {};
        swprintf_s(name, L"Command Allocator %d", i);
        this->CommandAllocators[i]->SetName(name);
#endif
    }
}

ComPtr<ID3D12GraphicsCommandList> Renderer::CreateCommandList()
{
    ComPtr<ID3D12CommandAllocator> commandAllocator = this->CommandAllocators[this->CurrentFrameBufferIndex];

    if (this->Device == nullptr || commandAllocator == nullptr)
    {
        this->Logger->Fatal("Renderer::CreateCommandList: Device or command allocator is null");
        return nullptr;
    }

    // Create command list
    ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

    HRESULT result = this->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    Platform::CheckHandle(result, "Failed to create command list");

    // Close by default
    commandList->Close();

    return commandList;
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
    swapChainDescription.BufferCount = this->SwapChainBufferCount;
    swapChainDescription.Width = this->FrameBufferWidth;
    swapChainDescription.Height = this->FrameBufferHeight;
    swapChainDescription.Format = this->FrameBufferFormat;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDescription.SampleDesc.Count = 1; // No anti-aliasing
    swapChainDescription.SampleDesc.Quality = 0;
    swapChainDescription.Scaling = DXGI_SCALING_STRETCH;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDescription = {0};
    swapChainFullScreenDescription.Windowed = TRUE;

    ComPtr<IDXGISwapChain1> swapChain = nullptr;

    result = this->DXGIFactory->CreateSwapChainForHwnd(this->CommandQueue.Get(), this->WindowHandle, &swapChainDescription, &swapChainFullScreenDescription, nullptr, &swapChain);
    Platform::CheckHandle(result, "Failed to create swap chain");

    if (FAILED(swapChain.As(&this->SwapChain)))
    {
        this->Logger->Fatal("Renderer::CreateSwapChain: Failed to cast swap chain to IDXGISwapChain3");
        return;
    }

    this->DXGIFactory->MakeWindowAssociation(this->WindowHandle, DXGI_MWA_NO_ALT_ENTER);

    this->CurrentFrameBufferIndex = this->SwapChain->GetCurrentBackBufferIndex();
}

void Renderer::CleanupRenderTargetViews()
{
    for (UINT i = 0; i < this->SwapChainBufferCount; i++)
    {
        this->RenderTargets[i].Reset();
        this->FrameFenceValues[i] = this->FrameFenceValues[this->CurrentFrameBufferIndex];
    }
}

void Renderer::CreateRenderTargetHeap()
{
    if (this->Device == nullptr || this->SwapChain == nullptr)
    {
        this->Logger->Fatal("Renderer::CreateRenderTargetView: Device or swap chain is null");
        return;
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescription = {};
    rtvHeapDescription.NumDescriptors = this->SwapChainBufferCount;
    rtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT result = this->Device->CreateDescriptorHeap(&rtvHeapDescription, IID_PPV_ARGS(&this->RTVHeap));

    Platform::CheckHandle(result, "Failed to create descriptor heap");
}

void Renderer::CreateRenderTargetViews()
{
    if (this->Device == nullptr || this->SwapChain == nullptr || this->RTVHeap == nullptr)
    {
        this->Logger->Fatal("Renderer::CreateRenderTargetView: Device, swap chain or RTV heap is null");
        return;
    }

    this->Logger->Message("Renderer::CreateRenderTargetView: Creating %u render target views", this->SwapChainBufferCount);

    HRESULT result;

    UINT rtvDescriptorSize = this->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = this->RTVHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < this->SwapChainBufferCount; i++)
    {
        result = this->SwapChain->GetBuffer(i, IID_PPV_ARGS(&this->RenderTargets[i]));
        Platform::CheckHandle(result, "Failed to get swap chain buffer");

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = this->FrameBufferFormat;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        this->Device->CreateRenderTargetView(this->RenderTargets[i].Get(), &rtvDesc, rtvHandle);

#ifdef BUILD_TYPE_DEBUG
        wchar_t name[25] = {};
        swprintf_s(name, L"RTV %u", i);
        this->RenderTargets[i]->SetName(name);
#endif

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

D3D12_RESOURCE_BARRIER Renderer::CreateTransitionBarrier(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource.Get();
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    return barrier;
}
