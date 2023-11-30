#include "../../Platform/Platform.h"

#include "RendererDirectX12.h"

RendererDirectX12::RendererDirectX12()
{
    this->Logger = Platform::GetLogger();
}

RendererDirectX12::~RendererDirectX12()
{
    // TODO: Not all resources are released yet
    // Release all DirectX 12 related resources

    if (this->CommandAllocator != nullptr)
    {
        this->CommandAllocator->Release();
        this->CommandAllocator = nullptr;
    }

    if (this->CommandQueue != nullptr)
    {
        this->CommandQueue->Release();
        this->CommandQueue = nullptr;
    }

    // if (this->CommandList != nullptr)
    // {
    //     this->CommandList->Release();
    //     this->CommandList = nullptr;
    // }

    this->CleanupRenderTargetViews();

    if (this->DXGIFactory != nullptr)
    {
        this->DXGIFactory->Release();
        this->DXGIFactory = nullptr;
    }

    if (this->SwapChain != nullptr)
    {
        this->SwapChain->Release();
        this->SwapChain = nullptr;
    }

    if (this->Device != nullptr)
    {
        this->Device->Release();
        this->Device = nullptr;
    }
}

void RendererDirectX12::Initialize(HWND windowHandle, UINT width, UINT height)
{
    this->WindowHandle = windowHandle;

    this->FrameBufferWidth = width;
    this->FrameBufferHeight = height;

    this->CreateDevice();
    this->CreateCommandInterfaces();
    this->CreateSwapChain();
    this->CreateRenderTargetViews();

    this->CreateFrameFence();

    this->TriangleCreateRootSignature();
    this->TriangleCompileShaders();
    this->TriangleCreatePipelineState();
    this->TriangleCreateVertexBuffer();
}

void RendererDirectX12::CreateFrameFence()
{
    // Create frame fence
    HRESULT result = this->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->FrameFence));

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12::CreateFrameFence: Failed to create frame fence");
        return;
    }
}

void RendererDirectX12::WaitForGPU()
{
    if (!this->CommandQueue || !this->FrameFence)
    {
        this->Logger->Fatal("RendererDirectX12::WaitForGPU: CommandQueue or FrameFence is null");
        return;
    }

    const UINT64 currentFenceValue = this->FrameFenceValues[this->CurrentFrameBufferIndex];
    HRESULT hr = this->CommandQueue->Signal(this->FrameFence.Get(), currentFenceValue);
    if (FAILED(hr))
    {
        this->Logger->Fatal("RendererDirectX12::WaitForGPU: Failed to signal command queue");
        return;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3 = nullptr;
    if (FAILED(this->SwapChain.As(&swapChain3)))
    {
        this->Logger->Fatal("RendererDirectX12::WaitForGPU: Failed to cast swap chain to IDXGISwapChain3");
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

void RendererDirectX12::Render()
{
    if (this->ShouldRender == false)
    {
        return;
    }

    // Ensure that the GPU is done rendering
    this->WaitForGPU();

    // Reset command allocator and command list
    this->CommandAllocator->Reset();
    this->CommandList->Reset(this->CommandAllocator.Get(), this->TrianglePipelineState.Get());

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

    // Set root signature
    this->CommandList->SetGraphicsRootSignature(this->TriangleRootSignature.Get());

    // Set vertex buffer
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = this->TriangleVertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(this->TriangleVertices);
    vertexBufferView.StrideInBytes = sizeof(Vertex);

    this->CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // Set primitive topology
    this->CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw triangle
    this->CommandList->DrawInstanced(3, 1, 0, 0);

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

void RendererDirectX12::TriangleCreateVertexBuffer()
{
    // Create vertex buffer
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDescription = {};
    resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDescription.Alignment = 0;
    resourceDescription.Width = sizeof(this->TriangleVertices);
    resourceDescription.Height = 1;
    resourceDescription.DepthOrArraySize = 1;
    resourceDescription.MipLevels = 1;
    resourceDescription.Format = DXGI_FORMAT_UNKNOWN;
    resourceDescription.SampleDesc.Count = 1;
    resourceDescription.SampleDesc.Quality = 0;
    resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT result = this->Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->TriangleVertexBuffer));

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12::TriangleCreateVertexBuffer: Failed to create vertex buffer");
        return;
    }

    // Copy vertex data to vertex buffer
    UINT8 *vertexDataBegin = nullptr;
    D3D12_RANGE readRange = {0, 0};

    result = this->TriangleVertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&vertexDataBegin));

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12::TriangleCreateVertexBuffer: Failed to map vertex buffer");
        return;
    }

    memcpy(vertexDataBegin, this->TriangleVertices, sizeof(this->TriangleVertices));

    this->TriangleVertexBuffer->Unmap(0, nullptr);
}

void RendererDirectX12::TriangleCreatePipelineState()
{
    D3D12_INPUT_ELEMENT_DESC inputElement[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };

    UINT inputElementCount = sizeof(inputElement) / sizeof(D3D12_INPUT_ELEMENT_DESC);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription = {};
    pipelineStateDescription.InputLayout = {inputElement, inputElementCount};
    pipelineStateDescription.pRootSignature = this->TriangleRootSignature.Get();
    pipelineStateDescription.VS = {reinterpret_cast<BYTE *>(this->TriangleVertexShader->GetBufferPointer()), this->TriangleVertexShader->GetBufferSize()};
    pipelineStateDescription.PS = {reinterpret_cast<BYTE *>(this->TrianglePixelShader->GetBufferPointer()), this->TrianglePixelShader->GetBufferSize()};

    D3D12_RASTERIZER_DESC rasterizerDescription = {};
    rasterizerDescription.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDescription.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDescription.FrontCounterClockwise = FALSE;
    rasterizerDescription.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDescription.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDescription.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDescription.DepthClipEnable = TRUE;
    rasterizerDescription.MultisampleEnable = FALSE;
    rasterizerDescription.AntialiasedLineEnable = FALSE;
    rasterizerDescription.ForcedSampleCount = 0;
    rasterizerDescription.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_BLEND_DESC blendDescription = {};
    blendDescription.AlphaToCoverageEnable = FALSE;
    blendDescription.IndependentBlendEnable = FALSE;
    blendDescription.RenderTarget[0].BlendEnable = FALSE;
    blendDescription.RenderTarget[0].LogicOpEnable = FALSE;
    blendDescription.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    blendDescription.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    blendDescription.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDescription.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDescription.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDescription.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDescription.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDescription.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_DEPTH_STENCIL_DESC depthStencilDescription = {};
    depthStencilDescription.DepthEnable = FALSE;
    depthStencilDescription.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDescription.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencilDescription.StencilEnable = FALSE;
    depthStencilDescription.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStencilDescription.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    depthStencilDescription.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthStencilDescription.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDescription.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDescription.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDescription.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthStencilDescription.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDescription.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDescription.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

    pipelineStateDescription.RasterizerState = rasterizerDescription;
    pipelineStateDescription.BlendState = blendDescription;
    pipelineStateDescription.DepthStencilState = depthStencilDescription;
    pipelineStateDescription.SampleMask = UINT_MAX;
    pipelineStateDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDescription.NumRenderTargets = 1;
    pipelineStateDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateDescription.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;

    HRESULT result = this->Device->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState));

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12::TriangleCreatePipelineState: Failed to create pipeline state");
        return;
    }

    this->TrianglePipelineState = pipelineState.Detach();
}

void RendererDirectX12::TriangleCompileShaders()
{
    UINT compileFlags = 0;

#ifdef BUILD_TYPE_DEBUG
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT result = D3DCompileFromFile(L"E:\\code\\Flying-Rat\\Rendering-101\\shaders\\TriangleVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &this->TriangleVertexShader, nullptr);

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12::TriangleCompileShaders: Failed to compile vertex shader");
        return;
    }

    result = D3DCompileFromFile(L"E:\\code\\Flying-Rat\\Rendering-101\\shaders\\TrianglePixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &this->TrianglePixelShader, nullptr);

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12::TriangleCompileShaders: Failed to compile pixel shader");
        return;
    }
}

void RendererDirectX12::TriangleCreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDescription = {};
    rootSignatureDescription.NumParameters = 0;
    rootSignatureDescription.pParameters = nullptr;
    rootSignatureDescription.NumStaticSamplers = 0;
    rootSignatureDescription.pStaticSamplers = nullptr;
    rootSignatureDescription.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> signature = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> error = nullptr;

    HRESULT result = D3D12SerializeRootSignature(&rootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12::TriangleCreateRootSignature: Failed to serialize root signature");
        return;
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

    result = this->Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12::TriangleCreateRootSignature: Failed to create root signature");
        return;
    }

    this->TriangleRootSignature = rootSignature.Detach();
}

void RendererDirectX12::Resize(UINT width, UINT height)
{
    if (this->SwapChain == nullptr)
    {
        this->Logger->Fatal("RendererDirectX12::Resize: Swap chain is null");
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

void RendererDirectX12::CreateDevice()
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
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory7 = nullptr;

    if (FAILED(this->DXGIFactory.As(&dxgiFactory7)))
    {
        this->Logger->Fatal("RendererDirectX12::FindSuitableHardwareAdapter: Failed to cast DXGI factory to DXGI factory 7");
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

            this->Logger->Message("RendererDirectX12::FindSuitableHardwareAdapter: Found hardware adapter: %ls", description);

            GPUPerformanceClass performanceClass = this->TryToDeterminePerformanceClass(&adapterDescription);

            switch (performanceClass)
            {
            case GPUPerformanceClass::Unknown:
                this->Logger->Message("RendererDirectX12::FindSuitableHardwareAdapter: Unknown performance class");
                break;
            case GPUPerformanceClass::Integrated:
                this->Logger->Message("RendererDirectX12::FindSuitableHardwareAdapter: Integrated performance class");
                break;
            case GPUPerformanceClass::Dedicated:
                this->Logger->Message("RendererDirectX12::FindSuitableHardwareAdapter: Dedicated performance class");
                break;
            }

            break;
        }
    }

    this->Adapter = currentAdapter.Detach();
}

void RendererDirectX12::CreateCommandInterfaces()
{
    if (this->Device == nullptr)
    {
        this->Logger->Fatal("RendererDirectX12::CreateCommandInterfaces: Device is null");
        return;
    }

    HRESULT result;

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDescription = {};
    queueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Compute and graphics commands

    result = this->Device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&this->CommandQueue));
    this->CheckHandle(result, "Failed to create command queue");

    // Create command allocator
    result = this->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->CommandAllocator));
    this->CheckHandle(result, "Failed to create command allocator");

    // Create command list
    result = this->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&this->CommandList));
    this->CheckHandle(result, "Failed to create command list");

    // Close command list
    this->CommandList->Close();
}

void RendererDirectX12::CreateSwapChain()
{
    if (this->DXGIFactory == nullptr || this->WindowHandle == nullptr || this->CommandQueue == nullptr)
    {
        this->Logger->Fatal("RendererDirectX12::CreateSwapChain: DXGI factory, window handle or command queue is null");
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
    this->CheckHandle(result, "Failed to cast DXGI factory to DXGI factory 7");

    result = dxgiFactory7->CreateSwapChainForHwnd(this->CommandQueue.Get(), this->WindowHandle, &swapChainDescription, nullptr, nullptr, &swapChain);
    this->CheckHandle(result, "Failed to create swap chain");

    this->SwapChain = swapChain.Detach();
}

void RendererDirectX12::CleanupRenderTargetViews()
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

void RendererDirectX12::CreateRenderTargetViews()
{
    if (this->Device == nullptr || this->SwapChain == nullptr)
    {
        this->Logger->Fatal("RendererDirectX12::CreateRenderTargetView: Device or swap chain is null");
        return;
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescription = {};
    rtvHeapDescription.NumDescriptors = this->BufferCount;
    rtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT result = this->Device->CreateDescriptorHeap(&rtvHeapDescription, IID_PPV_ARGS(&this->RTVHeap));

    this->CheckHandle(result, "Failed to create descriptor heap");

    UINT rtvDescriptorSize = this->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = this->RTVHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < this->BufferCount; i++)
    {
        result = this->SwapChain->GetBuffer(i, IID_PPV_ARGS(&this->RenderTargets[i]));
        this->CheckHandle(result, "Failed to get swap chain buffer");

        this->Device->CreateRenderTargetView(this->RenderTargets[i].Get(), nullptr, rtvHandle);

        rtvHandle.ptr += rtvDescriptorSize;
    }
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

void RendererDirectX12::CheckHandle(HRESULT result, const char *message, bool shouldCrash)
{
    if (FAILED(result))
    {
        this->Logger->Fatal("RendererDirectX12: %s", message);

        Platform::TriggerBreakpoint();

        if (shouldCrash)
        {
            Platform::TriggerCrash();
        }
    }
}
