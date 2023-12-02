#pragma once

#ifndef TRIANGLE_ENTITY_H
#define TRIANGLE_ENTITY_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <d3dcompiler.h>

#include "../Platform/Generic.h"
#include "../Platform/Platform.h"
#include "../Graphics/DirectX12/Vertex.h"
#include "../Graphics/DirectX12/Tools.h"

namespace Entities
{
    class TriangleEntity
    {
    private:
        Diagnostics::Logger *Logger = nullptr;

        struct ConstantBufferPayload
        {
            // "Time"
            float Time = 0.0f;
        };

        const Graphics::DirectX12::Vertex Vertices[3] = {
            {Graphics::DirectX12::Float3(0.0f, 0.25f, 0.0f), Graphics::DirectX12::Float4(1.0f, 0.0f, 0.0f, 1.0f)},
            {Graphics::DirectX12::Float3(0.25f, -0.25f, 0.0f), Graphics::DirectX12::Float4(0.0f, 1.0f, 0.0f, 1.0f)},
            {Graphics::DirectX12::Float3(-0.25f, -0.25f, 0.0f), Graphics::DirectX12::Float4(0.0f, 0.0f, 1.0f, 1.0f)}};

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
    };
}

#endif // TRIANGLE_ENTITY_H
