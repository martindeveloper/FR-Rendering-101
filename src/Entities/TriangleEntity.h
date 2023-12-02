#pragma once

#ifndef TRIANGLE_ENTITY_H
#define TRIANGLE_ENTITY_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <d3dcompiler.h>

#include "../Platform/Generic.h"
#include "../Platform/Platform.h"
#include "../Renderer/DirectX12/Vertex.h"

class TriangleEntity
{
private:
    Logger *Logger = nullptr;

    struct ConstantBufferPayload
    {
        // "Time"
        float Time = 0.0f;
    };

    const Vertex Vertices[3] = {
        {DirectX::XMFLOAT3(0.0f, 0.25f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
        {DirectX::XMFLOAT3(0.25f, -0.25f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
        {DirectX::XMFLOAT3(-0.25f, -0.25f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}};

    Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBuffer = nullptr;

public:
    TriangleEntity();

    void OnResourceCreate(Microsoft::WRL::ComPtr<ID3D12Device> device);
    void OnRender(UINT frame, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

private:
    void CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> device);
    void CreatePipelineState(Microsoft::WRL::ComPtr<ID3D12Device> device);
    void CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device);
    void CreateConstantBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device);
    void CreateShaders();

    inline void CheckShaderError(HRESULT result, ID3DBlob *blob, const char *message, bool shouldCrash = true);
};

#endif // TRIANGLE_ENTITY_H
