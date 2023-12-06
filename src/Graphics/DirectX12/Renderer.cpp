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
    // No extra need to manually release resources because of ComPtr

    this->WaitForGPU();

    CloseHandle(this->FrameFenceEvent);

    this->UnloadDiagnosticsModules();
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

    if (FAILED(result))
    {
        this->Logger->Fatal("Renderer::CreateFrameFence: Failed to create frame fence");
        return;
    }

    // Create frame fence event
    this->FrameFenceEvent = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
}

void Renderer::WaitBeforeNextFrame()
{
    if (!this->CommandQueue || !this->FrameFence)
    {
        this->Logger->Fatal("Renderer::WaitBeforeNextFrame: CommandQueue or FrameFence is null");
        return;
    }

    const UINT64 currentFenceValue = ++this->FrameFenceValues[this->CurrentFrameBufferIndex];
    HRESULT hr = this->CommandQueue->Signal(this->FrameFence.Get(), currentFenceValue);

    if (FAILED(hr))
    {
        this->Logger->Fatal("Renderer::WaitBeforeNextFrame: Failed to signal command queue");
        return;
    }

    this->CurrentFrameBufferIndex = this->SwapChain->GetCurrentBackBufferIndex();

    if (this->FrameFence->GetCompletedValue() < this->FrameFenceValues[this->CurrentFrameBufferIndex])
    {
        this->FrameFence->SetEventOnCompletion(this->FrameFenceValues[this->CurrentFrameBufferIndex], this->FrameFenceEvent);
        WaitForSingleObjectEx(this->FrameFenceEvent, INFINITE, FALSE);
    }
}

void Renderer::WaitForGPU() noexcept
{
    // Wait for previous frame to finish rendering is not best practice but it's fine for our purposes

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

    this->FrameFence->SetEventOnCompletion(currentFenceValue, this->FrameFenceEvent);

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

    this->Logger->Message("Renderer::Resize: Resizing to %dx%d", width, height);

    this->ShouldRender = false;

    // We must wait for both swap chain buffers to be released before resizing
    this->WaitForGPU();

    this->FrameBufferWidth = width;
    this->FrameBufferHeight = height;

    // Remove old render target views
    this->CleanupRenderTargetViews();

    this->SwapChain->ResizeBuffers(this->SwapChainBufferCount, width, height, this->FrameBufferFormat, 0);

    // Create new render target views with new buffer size
    this->CreateRenderTargetViews();

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
    ComPtr<IDXGIAdapter1> currentAdapter = nullptr;
    ComPtr<IDXGIFactory7> dxgiFactory7 = nullptr;

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

    // One queue is fine for two swap chain buffers
    result = this->Device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&this->CommandQueue));
    Platform::CheckHandle(result, "Failed to create command queue");

    this->CommandQueue->SetName(L"Command Queue Primary");

    for (UINT i = 0; i < this->SwapChainBufferCount; i++)
    {
        wchar_t name[25] = {};
        swprintf_s(name, L"Command Allocator %d", i);

        result = this->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->CommandAllocators[i]));
        Platform::CheckHandle(result, "Failed to create command allocator");

        this->CommandAllocators[i]->SetName(name);
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

    ComPtr<IDXGISwapChain1> swapChain = nullptr;

    ComPtr<IDXGIFactory7> dxgiFactory7 = nullptr;

    result = this->DXGIFactory.As(&dxgiFactory7);
    Platform::CheckHandle(result, "Failed to cast DXGI factory to DXGI factory 7");

    result = dxgiFactory7->CreateSwapChainForHwnd(this->CommandQueue.Get(), this->WindowHandle, &swapChainDescription, nullptr, nullptr, &swapChain);
    Platform::CheckHandle(result, "Failed to create swap chain");

    if (FAILED(swapChain.As(&this->SwapChain)))
    {
        this->Logger->Fatal("Renderer::CreateSwapChain: Failed to cast swap chain to IDXGISwapChain3");
        return;
    }

    dxgiFactory7->MakeWindowAssociation(this->WindowHandle, DXGI_MWA_NO_ALT_ENTER);

    this->CurrentFrameBufferIndex = this->SwapChain->GetCurrentBackBufferIndex();
}

void Renderer::CleanupRenderTargetViews()
{
    for (UINT i = 0; i < this->SwapChainBufferCount; i++)
    {
        this->RenderTargets[i].Reset();
        this->FrameFenceValues[i] = 1;
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
    rtvHeapDescription.NumDescriptors = this->SwapChainBufferCount;
    rtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT result = this->Device->CreateDescriptorHeap(&rtvHeapDescription, IID_PPV_ARGS(&this->RTVHeap));

    Platform::CheckHandle(result, "Failed to create descriptor heap");

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

        // Set name with index for debugging
        wchar_t name[25] = {};
        swprintf_s(name, L"SwapChainBuffer %u", i);
        this->RenderTargets[i]->SetName(name);

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
