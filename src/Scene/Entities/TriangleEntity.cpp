#include "TriangleEntity.h"

using namespace Scene::Entities;

TriangleEntity::TriangleEntity()
{
    this->Logger = Platform::GetLogger();
}

TriangleEntity::~TriangleEntity()
{
    this->Logger->Message("TriangleEntity: Destructor");
}

void TriangleEntity::OnShutdown()
{
    this->Logger->Message("TriangleEntity: Shutdown");

    delete this->VertexShaderBlob;
    delete this->PixelShaderBlob;

    // Reset all COM objects
    this->RootSignature.Reset();
    this->PipelineState.Reset();
    this->VertexBuffer.Reset();

    for (UINT i = 0; i < 2; i++)
    {
        this->ConstantBuffers[i].Reset();
    }
}

void TriangleEntity::OnResourceCreate(Graphics::DirectX12::ResourcesInitializationMetadata *resourceMetadata)
{
    this->CreateRootSignature(resourceMetadata->Device);
    this->CreateShaders();
    this->CreatePipelineState(resourceMetadata->Device);
    this->CreateVertexBuffer(resourceMetadata->Device);
    this->CreateConstantBuffers(resourceMetadata->Device, resourceMetadata->BackBufferCount);
}

void TriangleEntity::OnUpdate(uint64_t frame)
{
    this->Frame = frame;
}

void TriangleEntity::OnRender(Graphics::DirectX12::FrameMetadata *frameMetadata)
{
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = frameMetadata->CommandList;
    UINT currentBackBufferIndex = frameMetadata->CurrentBackBufferIndex;

    // Prepare constant buffer
    ConstantBufferPayload *constantBuffer = nullptr;
    this->ConstantBuffers[currentBackBufferIndex]->Map(0, nullptr, reinterpret_cast<void **>(&constantBuffer));

    // Not great, but we need some time-based animation
    constantBuffer->Time = static_cast<float>(this->Frame) / 100.0f;

    this->ConstantBuffers[currentBackBufferIndex]->Unmap(0, nullptr);

    // Set pipeline state
    commandList->SetPipelineState(this->PipelineState.Get());

    // Set root signature
    commandList->SetGraphicsRootSignature(this->RootSignature.Get());

    // Set vertex buffer
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = this->VertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(this->Vertices);
    vertexBufferView.StrideInBytes = sizeof(Graphics::DirectX12::Vertex);

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // Set constant buffer
    commandList->SetGraphicsRootConstantBufferView(0, this->ConstantBuffers[currentBackBufferIndex]->GetGPUVirtualAddress());

    // Set primitive topology
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw triangle
    commandList->DrawInstanced(3, 1, 0, 0);
}

void TriangleEntity::CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    D3D12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].Descriptor.RegisterSpace = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDescription = {};
    rootSignatureDescription.NumParameters = 1;
    rootSignatureDescription.pParameters = rootParameters;
    rootSignatureDescription.NumStaticSamplers = 0;
    rootSignatureDescription.pStaticSamplers = nullptr;
    rootSignatureDescription.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> signature = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> error = nullptr;

    HRESULT result = D3D12SerializeRootSignature(&rootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    Platform::CheckHandle(result, "Failed to serialize root signature");

    result = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&this->RootSignature));
    Platform::CheckHandle(result, "Failed to create root signature");

#ifdef BUILD_TYPE_DEBUG
    this->RootSignature->SetName(L"Triangle Root Signature");
#endif
}

void TriangleEntity::CreatePipelineState(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    D3D12_INPUT_ELEMENT_DESC inputElement[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };

    UINT inputElementCount = sizeof(inputElement) / sizeof(D3D12_INPUT_ELEMENT_DESC);

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

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription = {};
    pipelineStateDescription.InputLayout = {inputElement, inputElementCount};
    pipelineStateDescription.pRootSignature = this->RootSignature.Get();
    pipelineStateDescription.VS = {reinterpret_cast<BYTE *>(this->VertexShaderBlob->GetBufferPointer()), this->VertexShaderBlob->GetBufferSize()};
    pipelineStateDescription.PS = {reinterpret_cast<BYTE *>(this->PixelShaderBlob->GetBufferPointer()), this->PixelShaderBlob->GetBufferSize()};
    pipelineStateDescription.SampleMask = UINT_MAX;
    pipelineStateDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDescription.NumRenderTargets = 1;
    pipelineStateDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateDescription.SampleDesc.Count = 1;
    pipelineStateDescription.RasterizerState = rasterizerDescription;
    pipelineStateDescription.BlendState = blendDescription;
    pipelineStateDescription.DepthStencilState = depthStencilDescription;

    HRESULT result = device->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&this->PipelineState));
    Platform::CheckHandle(result, "Failed to create pipeline state");

#ifdef BUILD_TYPE_DEBUG
    this->PipelineState->SetName(L"Triangle Pipeline State");
#endif
}

void TriangleEntity::CreateShaders()
{
    UINT compileFlags = 0;

#ifdef BUILD_TYPE_DEBUG
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    const wchar_t *shaderVertexPath = SHADER_BYTECODE_PATH("Triangle", "Vertex");
    const wchar_t *shaderPixelPath = SHADER_BYTECODE_PATH("Triangle", "Pixel");

    this->Logger->Message("TriangleEntity: Loading vertex shader bytecode from %ls", shaderVertexPath);
    Graphics::DirectX12::Tools::LoadShaderByteCode(shaderVertexPath, &this->VertexShaderBlob);

    this->Logger->Message("TriangleEntity: Loading pixel shader bytecode from %ls", shaderPixelPath);
    Graphics::DirectX12::Tools::LoadShaderByteCode(shaderPixelPath, &this->PixelShaderBlob);
}

void TriangleEntity::CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device)
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
    resourceDescription.Width = sizeof(this->Vertices);
    resourceDescription.Height = 1;
    resourceDescription.DepthOrArraySize = 1;
    resourceDescription.MipLevels = 1;
    resourceDescription.Format = DXGI_FORMAT_UNKNOWN;
    resourceDescription.SampleDesc.Count = 1;
    resourceDescription.SampleDesc.Quality = 0;
    resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->VertexBuffer));
    Platform::CheckHandle(result, "Failed to create vertex buffer");

    // Copy vertex data to vertex buffer
    UINT8 *vertexDataBegin = nullptr;
    D3D12_RANGE readRange = {0, 0};

    result = this->VertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&vertexDataBegin));
    Platform::CheckHandle(result, "Failed to map vertex buffer");

    memcpy(vertexDataBegin, this->Vertices, sizeof(this->Vertices));

    this->VertexBuffer->Unmap(0, nullptr);

#ifdef BUILD_TYPE_DEBUG
    this->VertexBuffer->SetName(L"Triangle Vertex Buffer");
#endif
}

void TriangleEntity::CreateConstantBuffers(Microsoft::WRL::ComPtr<ID3D12Device> device, UINT backBufferCount)
{
    D3D12_RESOURCE_DESC constantBufferDesc = {};
    constantBufferDesc.Width = sizeof(ConstantBufferPayload);
    constantBufferDesc.Height = 1;
    constantBufferDesc.DepthOrArraySize = 1;
    constantBufferDesc.MipLevels = 1;
    constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    constantBufferDesc.SampleDesc.Count = 1;
    constantBufferDesc.SampleDesc.Quality = 0;
    constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    for (UINT i = 0; i < backBufferCount; i++)
    {
        device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->ConstantBuffers[i]));

#ifdef BUILD_TYPE_DEBUG
        wchar_t name[50];
        swprintf_s(name, L"Triangle Constant Buffer %d", i);
        this->ConstantBuffers[i]->SetName(name);
#endif
    }
}
